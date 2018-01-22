import xgboost as xgb
import numpy as np

# 1、xgBoost的基本使用
# 2、自定义损失函数的梯度和二阶导
# 3、binary:logistic/logitraw


# 定义f: theta * x
def log_reg(y_hat, y):
    p = 1.0 / (1.0 + np.exp(-y_hat))
    g = p - y.get_label()
    h = p * (1.0-p)
    return g, h


def error_rate(y_hat, y):
    return 'error', float(sum(y.get_label() != (y_hat > 0.5))) / len(y_hat)


if __name__ == "__main__":
    # 读取数据
    data_train = xgb.DMatrix('agaricus_train.txt')
    data_test = xgb.DMatrix('agaricus_test.txt')
    print data_train
    print type(data_train)

    #设置参数  
    #max_depth:树的最大深度,缺省值为6通常取值3-10  
  
    #eta:为了防止过拟合,更新过程中用到的收缩步长,在每次提升计算之后,算法会直接获得新特征的权重  
    #eta通过缩减特征的权重使得提升计算过程更加保守,默认值0.3  取值范围[0,1] 通常设置为[0.01-0.2]  
  
    #silent:取0时表示打印出运行时信息，取1时表示以缄默方式运行，不打印运行时信息。缺省值为0  
    #建议取0，过程中的输出数据有助于理解模型以及调参。另外实际上我设置其为1也通常无法缄默运行  
  
    #objective:缺省值 reg:linear 定义学习任务及相应的学习目标，可选目标函数如下：  
    # “reg:linear” –线性回归。  
    #“reg:logistic” –逻辑回归。  
    #“binary:logistic” –二分类的逻辑回归问题，输出为概率。  
    #“binary:logitraw” –二分类的逻辑回归问题，输出的结果为wTx。  
    #“count:poisson” –计数问题的poisson回归，输出结果为poisson分布,在poisson回归中，max_delta_step的缺省值为0  
    #“multi:softmax” –让XGBoost采用softmax目标函数处理多分类问题，同时需要设置参数num_class（类别个数）  
    #“multi:softprob” –和softmax一样，但是输出的是ndata * nclass的向量，可以将该向量reshape成ndata行nclass列的矩阵。没行数据表示样本所属于每个类别的概率。  
    param = {'max_depth': 3, 'eta': 1, 'silent': 1, 'objective': 'binary:logistic'} # logitraw
    # param = {'max_depth': 3, 'eta': 0.3, 'silent': 1, 'objective': 'reg:logistic'}
    watchlist = [(data_test, 'eval'), (data_train, 'train')]
    n_round = 7
    # bst = xgb.train(param, data_train, num_boost_round=n_round, evals=watchlist)
	 #xgboost 基本方法和默认参数  
    #函数原型:xgboost.train(params,dtrain,num_boost_round=10,evals=(),obj=None,feval=None,maximize=False,early_stopping_rounds=None,evals_result=None,verbose_eval=True,learning_rates=None,xgb_model=None)  
    # params  
    # 这是一个字典，里面包含着训练中的参数关键字和对应的值，形式是params = {‘booster’:’gbtree’, ’eta’:0.1}  
    # dtrain  
    # 训练的数据  
    # num_boost_round  
    # 这是指提升迭代的个数  
    # evals  
    # 这是一个列表，用于对训练过程中进行评估列表中的元素。形式是evals = [(dtrain,’train’), (dval,’val’)]或者是evals = [  
    #     (dtrain,’train’)], 对于第一种情况，它使得我们可以在训练过程中观察验证集的效果。  
    # obj, 自定义目的函数  
    # feval, 自定义评估函数  
    # maximize, 是否对评估函数进行最大化  
    # early_stopping_rounds, 早期停止次数 ，假设为100，验证集的误差迭代到一定程度在100次内不能再继续降低，就停止迭代。这要求evals  
    # 里至少有  
    # 一个元素，如果有多个，按最后一个去执行。返回的是最后的迭代次数（不是最好的）。如果early_stopping_rounds  
    # 存在，则模型会生成三个属性，bst.best_score, bst.best_iteration, 和bst.best_ntree_limit  
    # evals_result  
    # 字典，存储在watchlist  
    # 中的元素的评估结果。  
    # verbose_eval(可以输入布尔型或数值型)，也要求evals  
    # 里至少有  
    # 一个元素。如果为True, 则对evals中元素的评估结果会输出在结果中；如果输入数字，假设为5，则每隔5个迭代输出一次。  
    # learning_rates  
    # 每一次提升的学习率的列表，  
    # xgb_model, 在训练之前用于加载的xgb  
    # model。  
	
    bst = xgb.train(param, data_train, num_boost_round=n_round, evals=watchlist, obj=log_reg, feval=error_rate)

    # 计算错误率
    y_hat = bst.predict(data_test)
    y = data_test.get_label()
    print y_hat
    print y
    error = sum(y != (y_hat > 0.5))
    error_rate = float(error) / len(y_hat)
    print '样本总数：\t', len(y_hat)
    print '错误数目：\t%4d' % error
    print '错误率：\t%.5f%%' % (100*error_rate)
