﻿#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include<cmath>

using eosio::indexed_by;
using eosio::const_mem_fun;
using eosio::asset;
using eosio::permission_level;
using eosio::action;
using eosio::print;
using eosio::name;
using std::string;
using eosio::name;
using eosio::string_to_name;

class eosioshadows : public eosio::contract {
	
  const uint64_t INIT_BATCH = 100; // 每次分红处理人数
  const uint64_t INIT_WEIGHT = 7*24*60*60; // 权重奖池领取时间（7天）
  const uint64_t INIT_TIME = 1534075688; // 2018-08-12 20:08:08 启动游戏
  const uint64_t INIT_EOS = 100000000.0;//底仓股份1万，用于bancor定价，不能交易
  const uint64_t INIT_KEY = 10000000000.0;//总股本100万
  const std::string TEAM_ACCOUNT = "eosiodrizzle";//研发团队账号

  public:
    eosioshadows(account_name self):eosio::contract(self),users(_self, _self),games(_self, _self),bonuses(_self, _self){}
    
    void transfer( account_name from, account_name to, asset quantity, std::string memo ) {

        require_auth( from );
		
		//eosio_assert( now()<=0, "正在优化合约代码，请稍后买入"); 
        //eosio_assert( now()>=1533979200, "游戏在2018年8月11日下午5点30内测");    
        eosio_assert( now()>=INIT_TIME, "游戏在2018年8月12日晚上8点8分8秒启动游戏");  

        if(quantity.is_valid() && quantity.symbol == S(4, EOS) && from != _self && to == _self)
        {
            if(quantity.amount==1)
            {
                auto useritr = users.find( from );
                eosio_assert( useritr != users.end(), "账号不存在");
                eosio_assert( useritr->p >= 1000, "提取的利润不足0.1EOS");

                asset balance(useritr->p,S(4,EOS));
                users.modify( useritr, 0, [&]( auto& s ) {
                    s.p = 0;
                });

                action(
                permission_level{ _self, N(active) },
                N(eosio.token), N(transfer),
                std::make_tuple(_self,from, balance, std::string("简影游戏团队感谢你的支持：http://eosbao.io"))
                ).send();
				
            }else if(quantity.amount==2)
            {
                auto useritr = users.find( from );
                eosio_assert( useritr != users.end(), "账号不存在");
                eosio_assert( useritr->k >= 10000, "账号没有足够多的股份");

                auto gameitr = games.begin();
                eosio_assert( gameitr->k >= useritr->k, "发行的股份不足以出售");

                uint64_t eos = (INIT_EOS+gameitr->e) * useritr->k / (INIT_KEY-gameitr->k+useritr->k);
                eosio_assert( eos>0 && gameitr->e >= eos, "资金储备没有足够EOS");

                games.modify( gameitr, 0, [&]( auto& s ) {
                    s.k -= useritr->k;
                    s.e -= eos;
                });
                
                users.modify( useritr, 0, [&]( auto& s ) {
                    s.k = 0;
                    s.p += eos;
                    s.t = now();
                });

            }else if(quantity.amount >= 1000){

                eosio_assert( quantity.amount >= 10000, "购买数量必须大于等于1EOS" );
                eosio_assert( quantity.amount <= 100000*10000, "单次购买数量超出上限" );
                eosio_assert( memo.size() <= 256, "备注信息不能超过256位" );

                uint64_t eos = quantity.amount; 			           // 总的EOS
                uint64_t fee = eos*0.10;        			           // 10% 手续费给资源消耗及研发团队        
                uint64_t referrer = eos*0.05;   			           // 5% 给推荐人
                uint64_t weight = eos*0.15;     			           // 15% 权重奖池
                uint64_t jackpot = eos*0.50;  				 		   // 50% 储备资金用于Bancor定价 
				uint64_t distribute = eos-fee-referrer-weight-jackpot; // 20% 为持股用户直接分红

                auto gameitr = games.begin();
                if( gameitr == games.end() ) {
                    gameitr = games.emplace(_self,[&](auto& s)
                    {
                        s.i = 0;
                        s.e = 0;
                        s.k = 0;
                        s.f = 0;
                        s.w = 0;
                        s.r = 0;
                        s.d = 0;
                    });
                }
                
                uint64_t key = (INIT_KEY-gameitr->k) * jackpot / (INIT_EOS+gameitr->e+jackpot);
                eosio_assert( key>0 && key<=INIT_KEY, "股份数量不正确" );

                games.modify( gameitr, 0, [&]( auto& s ) {
                    s.e += jackpot;
                    s.k += key;
                    s.f += fee+distribute;
                    s.w += weight;
                    s.r += referrer;
					s.d += distribute;
                });

                auto teamAccount = string_to_name(TEAM_ACCOUNT.c_str());
                auto teamitr = users.find( teamAccount );
                if( teamitr == users.end())
                {
                    teamitr = users.emplace( _self, [&]( auto& s ) {
                        s.n = teamAccount;
                        s.r = 0;
                        s.e = 0;
                        s.k = 0;
                        s.p = 0;
                        s.t = now();
                    });
                }
                users.modify( teamitr,0, [&]( auto& s ) {
                    s.p += fee;
                });  
      
                auto useritr = users.find( from );
                if( useritr == users.end() ) {
                    auto parent = string_to_name(memo.c_str());
                    auto parentitr = users.find( parent );
                    if(memo.size()<=0 || memo.size()>12 || parent==_self || from==parent || parentitr==users.end())                 
                    {
                        parent = teamAccount;
                    }
                    if(from==teamAccount)
                    {
                        parent = 0;
                    }
                    useritr = users.emplace( _self, [&]( auto& s ) {
                        s.n = from;
                        s.r = parent;
                        s.e = eos;
                        s.k = key;
                        s.p = 0;
                        s.t = now();
                    });
                } else{
					uint64_t timespan = now()-useritr->t;
                    if(timespan>=INIT_WEIGHT && gameitr->w>10000)
                    {
                        double user_key = double(useritr->k);   
                        double game_key = double(gameitr->k); 
                        double time_ratio = double(timespan)/double(INIT_WEIGHT);
                        uint64_t weight_amount = uint64_t(floor(user_key/game_key*gameitr->w*time_ratio));

						if(weight_amount>gameitr->w*0.1)
						{
							weight_amount = gameitr->w*0.1;
						}
						
                        games.modify( gameitr, 0, [&]( auto& s ) {
                            s.w -= weight_amount;
                        });
						
                        users.modify( useritr,0, [&]( auto& s ) {
							s.e += eos;
							s.k += key;
                            s.p += weight_amount;
                            s.t = now()-eos/10000*60;
                        });
                    }else
					{
						users.modify( useritr,0, [&]( auto& s ) {
							s.e += eos;
							s.k += key;
							s.t = useritr->t - eos/10000*60;
						}); 
					}								
                }

                uint64_t profit_left = referrer;
                if(useritr->r>0)
                {
                    auto agent = users.find(useritr->r);
                    if(agent != users.end() )
                    {
                        users.modify( agent,0, [&]( auto& s ) {
                            s.p += referrer*0.80;
                        });
                        profit_left -= referrer*0.80;
                    }

                    if(agent->r>0){
                        auto big_agent = users.find(agent->r);
                        if(big_agent != users.end() )
                        {
                            users.modify( big_agent,0, [&]( auto& s ) {
                                s.p += referrer*0.20;
                            });
                            if(profit_left>=referrer*0.20)
                            {
                                profit_left -= referrer*0.20;
                            }
                        }
                    }
                    if(profit_left>0 && agent != users.end())
                    {
                        users.modify( agent,0, [&]( auto& s ) {
                            s.p += profit_left;
                        });
                    }
                }else
                {
                    users.modify( useritr,0, [&]( auto& s ) {
                        s.p += referrer;
                    });
                }
            }
        }

    }
    
    //@abi action
    void sell(account_name from, asset quantity)
    {
        require_auth( from );

        eosio_assert( quantity.is_valid(), "股份数量不正确");
        eosio_assert( quantity.amount >= 10000, "请至少出售1股" );
        eosio_assert( quantity.amount <= 1000000*10000, "卖出的股份超出上限" );

        auto useritr = users.find( from );
        eosio_assert( useritr != users.end(), "账号不存在");
        eosio_assert( useritr->k >= quantity.amount, "账号没有足够多的股份");

        auto gameitr = games.begin();
        uint64_t eos = (INIT_EOS+gameitr->e) * quantity.amount / (INIT_KEY-gameitr->k+quantity.amount);
        eosio_assert( gameitr->e >= eos, "资金储备没有足够多的EOS");
        eosio_assert( gameitr->k >= quantity.amount, "已售股份没有足够多的KEY");
        
        users.modify( useritr, 0, [&]( auto& s ) {
            s.k -= quantity.amount;
            s.p += eos;
            s.t = now();
        });

        games.modify( gameitr, 0, [&]( auto& s ) {
            s.k -= quantity.amount;
            s.e -= eos;
        });
    }


    //@abi action
    void jackpot()
    {
        auto gameitr = games.begin();
        eosio_assert( gameitr != games.end(), "系统数据不存在" );

        auto bonusitr = bonuses.begin();
        if(bonusitr==bonuses.end())
        {
            bonusitr = bonuses.emplace(_self,[&](auto& s)
            {
                s.n = 0;
                s.i = 0;
            });
        }
        eosio_assert( gameitr->d>10000, "分红已经分配完" );
        
        auto useritr =users.begin();
        if(bonusitr->n>0)
        {
            useritr = users.lower_bound(bonusitr->n);
        }

        uint64_t count = 0;
        for(;useritr!=users.end();++useritr)
        {
            if(useritr->n == bonusitr->n || useritr->k<10000) continue;

            double user_key = double(useritr->k);
            double game_key = double(gameitr->k);
            double game_bonus = double(gameitr->d);
            double ratio = user_key/game_key;

            users.modify( useritr,0, [&]( auto& s ) {
                s.p += uint64_t(floor(ratio*game_bonus));
            });

            if(count>INIT_BATCH)
            {
                bonuses.modify( bonusitr,0, [&]( auto& s ) {
                    s.n = useritr->n;
                });
                break;
            }
            count++;
        }

        if(useritr==users.end())
        {
            bonuses.modify( bonusitr,0, [&]( auto& s ) {
                s.n = 0;
            });
            games.modify( gameitr,0, [&]( auto& s ) {
                s.d = 0;
            });
        }
    }
    
	
  private:

    // @abi table games i64
    struct game{
      uint64_t i;
      uint64_t k;
      uint64_t e;
      uint64_t f;
      uint64_t w;
      uint64_t r;
      uint64_t d;

      uint64_t primary_key() const { return i; }
      EOSLIB_SERIALIZE(game, (i)(k)(e)(f)(w)(r)(d))
    };
    typedef eosio::multi_index<N(games), game> game_list;
    game_list games;
    
    // @abi table users i64
    struct user {
      account_name n;
      account_name r;
      uint64_t e;
      uint64_t k;
      int64_t p;
      uint64_t t;

      uint64_t primary_key() const { return n; }
      uint64_t get_key() const { return k; }
      EOSLIB_SERIALIZE(user, (n)(r)(e)(k)(p)(t))
    };
    typedef eosio::multi_index<N(users), user,
    indexed_by<N(k), const_mem_fun<user, uint64_t, &user::get_key>>
    > user_list;
    user_list users;
	
    // @abi table bonuses i64
    struct bonus{
      uint64_t i;
      account_name n;

      uint64_t primary_key() const { return i; }
      EOSLIB_SERIALIZE(bonus, (i)(n))
    };
    typedef eosio::multi_index<N(bonuses), bonus> bonus_list;
    bonus_list bonuses;
};

 #define EOSIO_ABI_EX( TYPE, MEMBERS ) \
 extern "C" { \
    void apply( uint64_t receiver, uint64_t code, uint64_t action ) { \
       if( action == N(onerror)) { \
          eosio_assert(code == N(eosio), "onerror action's are only valid from the \"eosio\" system account"); \
       } \
       auto self = receiver; \
       if((code == N(eosio.token) && action == N(transfer)) || (code == self && (action==N(sell) || action == N(jackpot) || action == N(onerror))) ) { \
          TYPE thiscontract( self ); \
          switch( action ) { \
             EOSIO_API( TYPE, MEMBERS ) \
          } \
       } \
    } \
 }

EOSIO_ABI_EX(eosioshadows, (transfer)(sell)(jackpot))