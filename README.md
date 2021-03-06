# 去中心化交易所
应用主要是利用Bancor算法实现的去中心化交易所，实现了EOS区块链上无需挂单、无对手盘的去中心化交易系统，完全通过智能合约算法自动对代币进行涨跌，省去了传统挂单买卖交易代币的复杂度。

## 智能合约

开发语言：C++

## 游戏规则

#### Bancor算法销售
* 使用Bancor算法销售和回收KEY，按当前资金储备中EOS数量定价，越早购买的KEY越便宜，购买KEY的玩家越多，价格越呈指数级增长，也可以随时卖出KEY。

#### 倒计时发布新游戏
* 新游戏分红是以买入KEY的形式回馈给持KEY玩家，无论当时KEY价多少，新游戏合约里50%利润自动转入当前合约买进KEY。

#### 推荐奖励
* 任何一位玩家购买KEY，都会向上2级贡献总资金5%的利润，然后再按80%、20%的比例划分给2个上级，直接上级功劳最大获得80%。

#### 分红奖池
* 每KEY销售的20%给所有已持KEY用户按持KEY比例分红，每30分钟分红一次，分红自动进入“我的利润”，可随时提现。
* `用户分红=持KEY数量/总售出KEY*分红奖池`

#### 权重奖池
* （1）权重奖励=持股数量/总售出股份 * 权重奖池 * 持股权重/7.
* （2）如果权重奖励大于（权重奖池*10%），则权重奖励等于权重奖池*10%。
* （3）持股权重大于等于7时，买进1EOS才能领取，领取后权重归零
