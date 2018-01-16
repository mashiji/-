import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from sklearn import svm
import matplotlib as mpl
from sklearn.model_selection import train_test_split
from sklearn.metrics import accuracy_score

# 'sepal length', 'sepal width','petal length','petal width'
iris_features=u'花萼长度', u'花萼宽度', u'花瓣长度', u'花瓣宽度'
data=pd.read_csv(r'C:\Users\mashiji\Desktop\iris.data',header=None)
x ,y= np.split(data,(4,),axis=1)
y=data[4]
y=pd.Categorical(y).codes
x=x.iloc[:,:2]  #取前两列
x_train,x_test,y_train,y_test=train_test_split(x,y,random_state=1,train_size=0.6)

#分类器
#clf=svm.SVC(C=0.5,kernel='linear',decision_function_shape='ovr')
clf=svm.SVC(C=1,kernel='rbf',gamma=30,decision_function_shape='ovr')
clf.fit(x_train,y_train.ravel())
 
#准确率    
print(clf.score(x_train,y_train))
print('训练集准确率：',accuracy_score(y_train,clf.predict(x_train)))
print(clf.score(x_test,y_test))
print('测试集准确率:',accuracy_score(y_test,clf.predict(x_test)))

#decision_function
print('decision function:\n',clf.decision_function(x_train))
print('\npredict:\n',clf.predict(x_train))

#画图
N, M = 100, 100  # 横纵各采样多少个值
x1_min, x2_min = x.min()
x1_max, x2_max = x.max()
t1 = np.linspace(x1_min, x1_max, N)
t2 = np.linspace(x2_min, x2_max, M)
x1, x2 = np.meshgrid(t1, t2)  # 生成网格采样点
grid_test = np.stack((x1.flat, x2.flat), axis=1)  # 测试点


print('grid_test=\n',grid_test)
Z=clf.decision_function(grid_test)
print(Z)

grid_hat=clf.predict(grid_test)
grid_hat=grid_hat.reshape(x1.shape)

mpl.rcParams['font.sans-serif'] = [u'SimHei']
mpl.rcParams['axes.unicode_minus'] = False

cm_light = mpl.colors.ListedColormap(['#A0FFA0', '#FFA0A0', '#A0A0FF'])
cm_dark = mpl.colors.ListedColormap(['g', 'r', 'b'])

plt.figure(facecolor='w')
plt.pcolormesh(x1,x2,grid_hat,cmap=cm_light)
plt.scatter(x[0],x[1],c=y,edgecolors='k',s=50,cmap=cm_dark)
plt.scatter(x_test[0],x_test[1],s=120,facecolors='none',zorder=10)
plt.xlabel(iris_features[0],fontsize=13)
plt.ylabel(iris_features[1],fontsize=13)
plt.xlim(x1_min, x1_max)
plt.ylim(x2_min, x2_max)
plt.title('鸢尾花SVM二分类',fontsize=16)
plt.show()
