# Asynchronous Control Flow Patterns with Callbacks
`Node.js`这类语言习惯于同步的编程风格，其`CPS`风格和异步特性的`API`是其标准，对于新手来说可能难以理解。编写异步代码可能是一种不同的体验，尤其是对异步控制流而言。异步代码可能让我们难以预测在`Node.js`中执行语句的顺序。例如读取一组文件，执行一串任务，或者等待一组操作完成，都需要开发人员采用新的方法和技术，以避免最终编写出效率低下和不可维护的代码。一个常见的错误是回调地狱，代码量急剧上升又不可读，使得简单的程序也难以阅读和维护。在本章中，我们将看到如何通过使用一些规则和一些模式来避免回调，并编写干净、可管理的异步代码。我们将看到控制流库，如`async`，可以极大地简化我们的问题，提升我们的代码可读性，更易于维护。

## 异步编程的困难
`JavaScript`中异步代码的顺序错乱无疑是很容易的。闭包和对匿名函数的定义可以使开发人员有更好的编程体验，而并不需要开发人员手动对异步操作进行管理和跳转。这是符合`KISS`原则的。简单且能保持异步代码控制流，让它在更短的时间内工作。但不幸的是，回调嵌套是以牺牲诸如模块性、可重用性和可维护性，增大整个函数的大小，导致糟糕的代码结构为代价的。大多数情况下，创建闭包在功能上是不需要的，但这更多是一种约束，而不是与异步编程相关的问题。认识到回调嵌套会使得我们的代码变得笨拙，然后根据最适合的解决方案采取相应的方法解决回调地狱，这是新手与专家的区别。

### 创建一个简单的Web爬虫
为了解释上述问题，我们创建了一个简单的Web爬虫，一个命令行应用，其接受一个`URL`为输入，然后可以把其内容下载到一个文件中。在下列代码中，我们会依赖以下两个`npm`库。

此外，我们还将引用一个叫做`./utilities`的本地模块。

我们的应用程序的核心功能包含在一个名为`spider.js`的模块中。如下所示，首先加载我们所需要的依赖包：

```javascript
const request = require('request');
const fs = require('fs');
const mkdirp = require('mkdirp');
const path = require('path');
const utilities = require('./utilities');
```

接下来，我们将创建一个名为`spider()`的新函数，该函数接受`URL`为参数，并在下载过程完成时调用一个回调函数。

```javascript
function spider(url, callback) {
  const filename = utilities.urlToFilename(url);
  fs.exists(filename, exists => {
    if (!exists) {
      console.log(`Downloading ${url}`);
      request(url, (err, response, body) => {
        if (err) {
          callback(err);
        } else {
          mkdirp(path.dirname(filename), err => {
            if (err) {
              callback(err);
            } else {
              fs.writeFile(filename, body, err => {
                if (err) {
                  callback(err);
                } else {
                  callback(null, filename, true);
                }
              });
            }
          });
        }
      });
    } else {
      callback(null, filename, false);
    }
  });
}
```

上述函数执行以下任务：

* 检查该`URL`的文件是否已经下载过，即验证相应文件是否已经被创建：

`fs.exists(filename, exists => ...`

* 如果文件还没有被下载，则执行下列代码进行下载操作：

`request(url, (err, response, body) => ...`

* 然后，我们需要确定目录下是否已经包含了该文件：

`mkdirp(path.dirname(filename), err => ...`

* 最后，我们把`HTTP`请求返回的报文主体写入文件系统：

`mkdirp(path.dirname(filename), err => ...`

要完成我们的`Web爬虫`应用程序，只需提供一个`URL`作为输入(在我们的例子中，我们从命令行参数中读取它)，我们只需调用`spider()`函数即可。

```javascript
spider(process.argv[2], (err, filename, downloaded) => {
  if (err) {
    console.log(err);
  } else if (downloaded) {
    console.log(`Completed the download of "${filename}"`);
  } else {
    console.log(`"${filename}" was already downloaded`);
  }
});
```

现在，我们开始尝试运行`Web爬虫`应用程序，但是首先，确保已有`utilities.js`模块和`package.json`中的所有依赖包已经安装到你的项目中：

```bash
npm install
```

之后，我们执行我们这个爬虫模块来下载一个网页，使用以下命令：

```bash
node spider http://www.example.com
```

我们的`Web爬虫`应用程序要求在我们提供的`URL`中总是包含协议类型(例如，`http://`)。另外，不要期望`HTML`链接被重新编写，也不要期望下载像图片这样的资源，因为这只是一个简单的例子来演示异步编程是如何工作的。

![](http://oczira72b.bkt.clouddn.com/17-10-11/6194673.jpg)

### 回调地狱
看看我们的`spider()`函数，我们可以发现，尽管我们实现的算法非常简单，但是生成的代码有几个级别的缩进，而且很难读懂。使用阻塞式的同步`API`实现类似的功能是很简单的，而且很少有机会让它看起来如此错误。然而，使用异步`CPS`是另一回事，使用闭包可能会导致出现难以阅读的代码。

大量闭包和回调将代码转换成不可读的、难以管理的情况称为回调地狱。它是`Node.js`中最受认可和最严重的反模式之一。一般来说，对于`JavaScript`而言。受此问题影响的代码的典型结构如下:

```javascript
asyncFoo(err => {
  asyncBar(err => {
    asyncFooBar(err => {
      //...
    });
  });
});
```

我们可以看到，用这种方式编写的代码是如何形成金字塔形状的，由于深嵌的原因导致的难以阅读，称为“末日金字塔”。

像前面的代码片段这样的代码最明显的问题是可读性差。由于嵌套太深，几乎不可能跟踪回调函数的结束位置和另一个回调函数开始的位置。

另一个问题是由每个作用域中使用的变量名的重叠引起的。通常，我们必须使用类似甚至相同的名称来描述变量的内容。最好的例子是每个回调接收到的错误参数。有些人经常尝试使用相同名称的变体来区分每个范围内的对象，例如，`error`、`err`、`err1`、`err2`等等。另一些人则倾向于隐藏在范围中定义的变量，总是使用相同的名称。例如，`err`。这两种选择都远非完美，而且会造成混淆，并增加导致`bug`的可能性。

此外，我们必须记住，虽然闭包在性能和内存消耗方面的代价很小。此外，它们还可以创建不易识别的内存泄漏，因为我们不应该忘记，由闭包引用的任何上下文变量都不会被垃圾收集所保留。

关于对于`V8`的闭包工作，可以参考[Vyacheslav Egorov的博客文章](http://mrale.ph/blog/2012/09/23/grokking-v8-closures-for-fun.html)。

如果我们看一下我们的`spider()`函数，我们会清楚地注意到它便是一个典型的回调地狱的场景，并且在这个函数中有我们刚才描述的所有问题。这正是我们将在本章中学习的模式和技巧所要解决的问题。

## 使用简单的JavaScript
既然我们已经遇到了第一个回调地狱的例子，我们知道我们应该避免什么。然而，在编写异步代码时，这并不是惟一的关注点。事实上，有几种情况下，控制一组异步任务的流需要使用特定的模式和技术，特别是如果我们只使用普通的`JavaScript`而没有任何外部库的帮助的情况下。例如，通过按顺序应用异步操作来遍历集合并不像在数组中调用`forEach()`那样简单，但实际上它需要一种类似于递归的技术。

在本节中，我们将学习如何避免回调地狱，以及如何使用简单的`JavaScript`实现一些最常见的控制流模式。

### 回调函数的准则