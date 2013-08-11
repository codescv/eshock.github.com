---
layout: post
title: "关于python decorators"
date: 2013-08-11 21:15
comments: true
categories: python
---

## 高阶函数
在python里面decorator看起来是个挺高端大气上档次的词, 不过这个东西在函数式语言里面其实很稀松平常, 用一句话概括起来就是高阶函数的语法糖.
什么叫高阶函数(high order procedures)呢? 就是返回值或者参数是函数的函数. 比如我们现在有个函数`hello_world`:

{% codeblock lang:python %}
def hello_word():
    print "hello world!"
{% endcodeblock %}

这个函数的个功能是打印一行字"hello world". 现在我们想写一个函数, 它的功能是把`hello_word`这行字打印10遍, 怎么做呢? 简单:

{% codeblock lang:python %}
def hello_world_10():
    for i in range(10):
        hello_world()
{% endcodeblock %}

现在我们站的更高一点, 希望写一个更通用的函数, 可以将任意函数执行任意10次, 要怎么办呢?

{% codeblock lang:python %}
def ten_times(func):
    def new_fun():
        for i in range(10):
            func()
    return new_fun

hello_world_10 = ten_times(hello_world)
hello_world_10()
{% endcodeblock %}

可以看到, `ten_times`这个函数比我们一般的函数更"高级"一些, 它接受的参数是一个函数, 把这个函数进行一些加工(执行10次)后, 返回一个新的函数.
这个函数就是高阶函数.

## decorator
理解了高阶函数后, 我们就可以看看decorator了. 我们把前一个例子改为:

{% codeblock lang:python %}
@ten_times
def hello_world_10():
    print "hello world!"
{% endcodeblock %}

这样, 新的`hello_world_10`和原来的版本完全等价. 所以说, `@ten_times`其实是以下代码的语法糖:

{% codeblock lang:python %}
def hello_world_10():
    print "hello world!"
hello_world_10 = ten_times(hello_world_10)
{% endcodeblock %}

## 原函数带参数
假设我们有如下`add`函数:

{% codeblock lang:python %}
def add(a, b):
    return a+b
{% endcodeblock %}

这一次, 我们的`ten_times`函数希望把原函数的_返回值_乘以10, 怎么做呢?

{% codeblock lang:python %}
def ten_times(func):
    def new_func(*args, **kwargs):
        return 10 * func(*args, **kwargs)
    return new_func

@ten_times
def add_10(a, b):
    return a+b
{% endcodeblock %}

由此可见, 我们在decorator的定义中, 将新函数的输入传递给原函数即可.

## decorator带参数
这次我们想把`ten_times`写的更加general一些, 我们希望它可以接受一个参数`times`, 可以将任意函数执行的结果乘以任意次. 首先我们用高阶函数的方式:

{% codeblock lang:python %}
def times(time):
    def times_f(func):
        def new_func(*args, **kwargs):
            return time * func(*args, **kwargs)
        return new_func
    return times_f

def add(a, b):
    return a+b

ten_times = times(10)
add_10 = ten_times(add)
{% endcodeblock %}

现在有点复杂了, 对吧? `times`首先接受一个参数`time`, 返回一个函数`times_f`, 这个`times_f`函数接受一个函数`func`作为参数, 然后又返回一个新的函数`new_func`. 换句话说, 这个函数返回了一个函数, 后者又返回一个函数.
注意到`new_func`引用了`times_f`中的局部变量`func`, 这样的函数被称为闭包(closure). 用decorator的语法糖要怎么写呢?

{% codeblock lang:python %}
@times(10)
def add_10(a, b):
    return a+b
{% endcodeblock %}

从之前的经验我们可以看到, `ten_times`这个函数需要返回一个函数, 所以这里我们知道`times(10)`也是一个函数,它的返回值是一个函数, 由此可知`times`是一个返回(返回函数的函数)的函数, 也印证了前面的分析.

{% codeblock lang:python %}
@times(10)
def add_10(a, b):
    return a+b
{% endcodeblock%}

## 一点小问题
我们还是回到比较简单的`ten_times`这个函数:

{% codeblock lang:python %}
@ten_times
def add(a, b):
    return a+b
 
>>> add.__name__
'new_func'
{% endcodeblock %}

我们看到, `add`的`__name__`属性变成了`new_func`! 仔细想想, 这很正常, 因为ten_times返回的函数就是`new_func`呀! 我们先暴力fix一下:

{% codeblock lang:python %}
def ten_times(func):
    def new_func(*args, **kwargs):
        return 10 * func(*args, **kwargs)
    new_func.__name__ = func.__name__
    return new_func

@ten_times
def add_10(a,b):
    return a+b

>>> add_10
<function add_10 at 0x10da62140>
{% endcodeblock %}

的确, 这个问题解决了. 不过, 还有`__doc__`等等属性呢?  每次声明decorator的时候都要这么搞一下, 不蛋疼么?
是的, 我们可以把这些fix的代码放到一起:

{% codeblock lang:python %}
def fix_props(func):
    def wrapper(f):
        def new_f(*args, **kwargs):
            return f(*args, **kwargs)
        new_f.__name__ = func.__name__
        new_f.__doc__ = func.__doc__
        return new_f
    return wrapper


def ten_times(func):
    @fix_props(func)
    def new_func(*args, **kwargs):
        return 10 * func(*args, **kwargs)
    return new_func

{% endcodeblock %}

这样, 就帮我们完成了这些琐碎的小问题. `fix_props`这个decorator可以将原函数的属性复制到新的wrapper函数, 使得原函数的docstring, name等不会丢失.
万一python升级了, 函数又有新的属性了, 肿么办?幸运的是, python里面有专门的库函数可以解决这个问题:
{% codeblock lang:python %}
from functools import wraps

def ten_times(func):
    @wraps(func)
    def new_func(*args, **kwargs):
        return 10 * func(*args, **kwargs)
    return new_func
{% endcodeblock %}

这个`wraps`的原理就和我们上面的`fix_props`类似.

## 应用
decorator在python里有挺多应用的, 最典型的是`@classmethod`和`@property`. 例如:

{% codeblock lang:python %}
class A(object):
    def __init__(self):
        self.__x = 0

    @property
    def x(self):
        return self.__x

    @x.setter
    def x(self, xval):
        self.__x = xval

    @classmethod
    def my_name(cls):
        return "A"

>>> A.my_name()
'A'
>>> a = A()
>>> a.x = 1
>>> a.x
1

{% endcodeblock %}
