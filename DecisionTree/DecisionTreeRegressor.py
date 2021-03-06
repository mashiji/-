import numpy as np
import matplotlib.pyplot as plt
from sklearn.tree import DecisionTreeRegressor

if __name__=="__main__":
    N=100
    x=np.random.rand(N)*6-3
    x.sort()
    y=np.sin(x)+np.random.randn(N)*0.05
    x=x.reshape(-1,1)
    print(x.shape)
    
    dt=DecisionTreeRegressor(criterion='mse',max_depth=9)
    dt.fit(x,y)
    x_test=np.linspace(-3,3,50).reshape(-1,1)
    y_hat=dt.predict(x_test)
    plt.plot(x,y,'r*',ms=10,label='Actual')
    plt.plot(x_test,y_hat,'g-',linewidth=2,label='Predict')
    plt.legend(loc='upper left')
    plt.show()
    
    depth=[2,4,6,8,10]
    clr='rgbmy'
    dtr=DecisionTreeRegressor(criterion='mse')
    plt.plot(x,y,'ko',ms=6,label='Actual')
    x_test=np.linspace(-3,3,50).reshape(-1,1)
    for d,c in zip(depth,clr):
        dtr.set_params(max_depth=d)
        dtr.fit(x,y)
        y_hat=dtr.predict(x_test)
        plt.plot(x_test,y_hat,'-',color=c,linewidth=2,label='Depth=%d' %d)
    plt.legend(loc='upper left')
    plt.grid(b=True)
    plt.show()
