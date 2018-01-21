import numpy as np
import xgboost as xgb

dtrain=xgb.DMatrix(r'C:\Users\mashiji\Desktop\agaricus.txt.train')
dtest=xgb.DMatrix(r'C:\Users\mashiji\Desktop\agaricus.txt.test')
watchlist=[(dtest,'eval'),(dtrain,'train')]

param={'max_depth':2,'eta':1,'silent':1,'objective':'binary:logistic'}
bst=xgb.train(param,dtrain,1,watchlist)
ptrain=bst.predict(dtrain,output_margin=True)
ptest=bst.predict(dtest,output_margin=True)
dtrain.set_base_margin(ptrain)
dtest.set_base_margin(ptest)



import xgboost as xgb
dtrain=xgb.DMatrix(r'C:\Users\mashiji\Desktop\agaricus.txt.train')
dtest=xgb.DMatrix(r'C:\Users\mashiji\Desktop\agaricus.txt.test')
param=[('max_depth',2),('objective','binary:logistic'),('eval_metric','logloss'),('eval_metric','error')]
num_round=2
watchlist=[(dtest,'eval'),(dtrain,'train')]

evals_result = {}
bst = xgb.train(param, dtrain, num_round, watchlist, evals_result=evals_result)
print('Access logloss metric directly from evals_result:')
print(evals_result['eval']['logloss'])

print('')
print('Access metrics through a loop:')
for e_name, e_mtrs in evals_result.items():
    print('- {}'.format(e_name))
    for e_mtr_name, e_mtr_vals in e_mtrs.items():
        print('   - {}'.format(e_mtr_name))
        print('      - {}'.format(e_mtr_vals))

print('')
print('Access complete dictionary:')
print(evals_result)
