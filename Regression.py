import numpy as np
import pandas as pd
from sklearn import preprocessing
from sklearn.linear_model import LogisticRegression
from sklearn.preprocessing import StandardScaler,PolynomialFeatures
from sklearn.pipeline import Pipeline
import matplotlib.pyplot as plt
import matplotlib as mpl
import matplotlib.patches as mpatches

if __name__=="__main__":
    path=r'C:\Users\mashiji\Desktop\iris.data'
#     df=pd.read_csv(path,header=None)
#     x=df.values[:,:-1]
#     y=df.values[:,-1]
#     le=preprocessing.LabelEncoder()
#     le.fit(['Iris-setosa', 'Iris-versicolor', 'Iris-virginica'])
#     y=le.transform(y)
#     print(y)
    data=pd.read_csv(path,header=None)
    data[4]=pd.Categorical(data[4]).codes
    x,y=np.split(data,(4,),axis=1)
    x=x.iloc[:,:2]
    x=np.array(x)
    y=np.array(y)
    lr=Pipeline([('sc',StandardScaler()),
                ('poly',PolynomialFeatures(degree=2)),
                ('clf',LogisticRegression())])
    lr.fit(x,y)
    y_hat=lr.predict(x)
    y_hat_prob=lr.predict_proba(x)
    np.set_printoptions(suppress=True)
    print(y_hat)
    print(y_hat_prob)
    print(u'准确度：%.2f%%' % (100*np.mean(y_hat == y.ravel())))
    
    N, M = 500, 500     # 横纵各采样多少个值
    x1_min, x1_max = x[:, 0].min(), x[:, 0].max()   # 第0列的范围
    x2_min, x2_max = x[:, 1].min(), x[:, 1].max()   # 第1列的范围
    t1 = np.linspace(x1_min, x1_max, N)
    t2 = np.linspace(x2_min, x2_max, M)
    x1, x2 = np.meshgrid(t1, t2)                    # 生成网格采样点
    x_test = np.stack((x1.flat, x2.flat), axis=1)  
    
    mpl.rcParams['font.sans-serif'] = [u'simHei']
    mpl.rcParams['axes.unicode_minus'] = False
    cm_light = mpl.colors.ListedColormap(['#77E0A0', '#FF8080', '#A0A0FF'])
    cm_dark = mpl.colors.ListedColormap(['g', 'r', 'b'])
    y_hat=lr.predict(x_test)
    y_hat=y_hat.reshape(x1.shape)
    plt.figure(facecolor='w')
    plt.pcolormesh(x1,x2,y_hat,cmap=cm_light)
    plt.scatter(x[:,0],x[:,1],c=y,edgecolors='k',s=50,cmap=cm_dark)
    plt.xlabel(u'花萼长度', fontsize=14)
    plt.ylabel(u'花萼宽度', fontsize=14)
    plt.xlim(x1_min, x1_max)
    plt.ylim(x2_min, x2_max)
    plt.grid()
    patchs = [mpatches.Patch(color='#77E0A0', label='Iris-setosa'),
              mpatches.Patch(color='#FF8080', label='Iris-versicolor'),
              mpatches.Patch(color='#A0A0FF', label='Iris-virginica')]
    plt.legend(handles=patchs, fancybox=True, framealpha=0.8)
    plt.title(u'鸢尾花Logistic回归分类效果 - 标准化', fontsize=17)
    plt.show()
