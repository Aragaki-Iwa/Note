## 高级操作
1.callback  
    在求解过程中需要实现一些功能，如获取信息、终止优化、cut、嵌入自己的算法等。  
2.常用的线性化方法  
    max/min objective function  
    带fixed cost obj --分段式obj  
    逻辑  
    partial integer variable  
---------
Callback函数

    定义callback函数：def 函数名(model, where):...
    调用callback函数：m.optimize(callback函数名)
    
    使用callback函数注意两个参数：
    1.where:回调函数触发点
    2.what:获取何种信息，what能获取什么取决于where，使用时一定要正确对应二者关系
    
