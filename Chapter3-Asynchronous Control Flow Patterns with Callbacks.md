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

关于对于`V8`的闭包工作原理，可以参考[Vyacheslav Egorov的博客文章](http://mrale.ph/blog/2012/09/23/grokking-v8-closures-for-fun.html)。

如果我们看一下我们的`spider()`函数，我们会清楚地注意到它便是一个典型的回调地狱的场景，并且在这个函数中有我们刚才描述的所有问题。这正是我们将在本章中学习的模式和技巧所要解决的问题。

## 使用简单的JavaScript
既然我们已经遇到了第一个回调地狱的例子，我们知道我们应该避免什么。然而，在编写异步代码时，这并不是惟一的关注点。事实上，有几种情况下，控制一组异步任务的流需要使用特定的模式和技术，特别是如果我们只使用普通的`JavaScript`而没有任何外部库的帮助的情况下。例如，通过按顺序应用异步操作来遍历集合并不像在数组中调用`forEach()`那样简单，但实际上它需要一种类似于递归的技术。

在本节中，我们将学习如何避免回调地狱，以及如何使用简单的`JavaScript`实现一些最常见的控制流模式。

### 回调函数的准则
在编写异步代码时，要记住的第一个规则是在定义回调时不要滥用闭包。滥用闭包一时很爽，因为它不需要对诸如模块化和可重用性这样的问题进行额外的思考。但是，我们已经看到，这种做法弊大于利。大多数情况下，修复回调地狱问题并不需要任何库、花哨的技术或范式的改变，只是一些常识。

以下是一些基本原则，可以帮助我们更少的嵌套，并改进我们的代码的组织：

* 尽可能退出外层函数。根据上下文，使用`return`、`continue`或`break`，以便立即退出当前代码块，而不是使用`if...else`代码块。其他语句。这将有助于优化我们的代码结构。
* 为回调创建命名函数，避免使用闭包，并将中间结果作为参数传递。命名函数也会使它们在堆栈跟踪中更优雅。
* 代码尽可能模块化。并尽可能将代码分成更小的、可重用的函数。

### 回调调用的准则
为了展示上述原则，我们通过重构`Web爬虫`应用程序来说明。

对于第一步，我们可以通过删除`else`语句来重构我们的错误检查方式。这是在我们收到错误后立即从函数中返回。因此，看以下代码：

```javascript
if (err) {
  callback(err);
} else {
  // 如果没有错误，执行该代码块
}
```

我们可以通过编写下面的代码来改进我们的代码结构：

```javascript
if (err) {
  return callback(err);
}
// 如果没有错误，执行该代码块
```

有了这个简单的技巧，我们立即减少了函数的嵌套级别，它很简单，不需要任何复杂的重构。

在执行我们刚才描述的优化时，一个常见的错误是在调用回调函数之后忘记终止函数，即`return`。对于错误处理场景，以下代码是`bug`的典型来源：

```javascript
if (err) {
  callback(err);
}
// 如果没有错误，执行该代码块
```

在这个例子中，即使在调用回调之后，函数的执行也会继续。那么避免这种情况的出现，`return`语句是十分必要的。还要注意，函数返回的输出是什么并不重要，实际结果(或错误)是异步生成的，并传递给回调。异步函数的返回值通常被忽略。该属性允许我们编写如下的代码：

```javascript
return callback(...);
```

否则我们必须拆成两条语句来写：

```javascript
callback(...);
return;
```

接下来我们继续重构我们的`spider()`函数，我们可以尝试识别可复用的代码片段。例如，将给定字符串写入文件的功能可以很容易地分解为一个单独的函数：

```javascript
function saveFile(filename, contents, callback) {
  mkdirp(path.dirname(filename), err => {
    if (err) {
      return callback(err);
    }
    fs.writeFile(filename, contents, callback);
  });
}
```

遵循同样的原则，我们可以创建一个名为`download()`的通用函数，它将`URL`和`文件名`作为输入，并将`URL`的内容下载到给定的文件中。在内部，我们可以使用前面创建的`saveFile()`函数。

```javascript
function download(url, filename, callback) {
  console.log(`Downloading ${url}`);
  request(url, (err, response, body) => {
    if (err) {
      return callback(err);
    }
    saveFile(filename, body, err => {
      if (err) {
        return callback(err);
      }
      console.log(`Downloaded and saved: ${url}`);
      callback(null, body);
    });
  });
}
```

最后，修改我们的`spider()`函数：

```javascript
function spider(url, callback) {
  const filename = utilities.urlToFilename(url);
  fs.exists(filename, exists => {
    if (exists) {
      return callback(null, filename, false);
    }
    download(url, filename, err => {
      if (err) {
        return callback(err);
      }
      callback(null, filename, true);
    })
  });
}
```

`spider()`函数的功能和接口仍然是完全相同的，改变的仅仅是代码的组织方式。通过应用上述基本原则，我们能够极大地减少代码的嵌套，同时增加了它的可重用性和可测试性。实际上，我们可以考虑导出`saveFile()`和`download()`，这样我们就可以在其他模块中重用它们。这也使我们能够更容易地测试他们的功能。

我们在这一节中进行的重构清楚地表明，大多数时候，我们所需要的只是一些规则，并确保我们不滥用闭包和匿名函数。它的工作非常出色，只需最少的工作量，并且只使用原始的`JavaScript`。

### 顺序执行
现在开始探寻异步控制流的执行顺序，我们会通过开始分析一串异步代码来探寻其控制流。

按顺序执行一组任务意味着一次一个接一个地运行它们。执行顺序很重要，必须保证其正确性，因为列表中一个任务的结果可能会影响下一个任务的执行。下图说明了这个概念：

![](http://oczira72b.bkt.clouddn.com/17-10-14/39602217.jpg)

上述异步控制流有一些不同的变化：

* 按顺序执行一组已知任务，无需链接或传递执行结果
* 使用任务的输出作为下一个输入（也称为`chain`，`pipeline`，或者`waterfall`）
* 在每个元素上运行异步任务时迭代一个集合，一个元素接一个元素

对于顺序执行而言，尽管在使用直接样式阻塞`API`实现很简单，但通常情况下使用`异步CPS`时会导致回调地狱问题。

#### 按顺序执行一组已知的任务
在上一节中实现`spider()`函数时，我们已经遇到了顺序执行的问题。通过研究如下方式，我们可以更好地控制异步代码。以该代码为准则，我们可以用以下模式来解决上述问题：

```javascript
function task1(callback) {
  asyncOperation(() => {
    task2(callback);
  });
}

function task2(callback) {
  asyncOperation(result() => {
    task3(callback);
  });
}

function task3(callback) {
  asyncOperation(() => {
    callback(); //finally executes the callback
  });
}

task1(() => {
  //executed when task1, task2 and task3 are completed
  console.log('tasks 1, 2 and 3 executed');
});
```

上述模式显示了在完成一个异步操作后，再调用下一个异步操作。该模式强调任务的模块化，并且避免在处理异步代码使用闭包。

#### 顺序迭代
我们前面描述的模式如果我们预先知道要执行什么和有多少个任务，这些模式是完美的。这使我们能够对序列中下一个任务的调用进行硬编码，但是如果要对集合中的每个项目执行异步操作，会发生什么？在这种情况下，我们不能对任务序列进行硬编码。相反的是，我们必须动态构建它。

##### Web爬虫版本2
为了显示顺序迭代的例子，让我们为`Web爬虫`应用程序引入一个新功能。我们现在想要递归地下载网页中的所有链接。要做到这一点，我们将从页面中提取所有链接，然后按顺序逐个地触发我们的`Web爬虫`应用程序。

第一步是修改我们的`spider()`函数，以便通过调用一个名为`spiderLinks()`的函数触发页面所有链接的递归下载。

此外，我们现在尝试读取文件，而不是检查文件是否已经存在，并开始爬取其链接。这样，我们就可以恢复中断的下载。最后还有一个变化是，我们确保我们传递的参数是最新的，还要限制递归深度。结果代码如下：

```javascript
function spider(url, nesting, callback) {
  const filename = utilities.urlToFilename(url);
  fs.readFile(filename, 'utf8', (err, body) => {
    if (err) {
      if (err.code! == 'ENOENT') {
        return callback(err);
      }
      return download(url, filename, (err, body) => {
        if (err) {
          return callback(err);
        }
        spiderLinks(url, body, nesting, callback);
      });
    }
    spiderLinks(url, body, nesting, callback);
  });
}
```

##### 爬取链接
现在我们可以创建这个新版本的`Web爬虫`应用程序的核心，即`spiderLinks()`函数，它使用顺序异步迭代算法下载`HTML`页面的所有链接。注意我们在下面的代码块中定义的方式：

```javascript
function spiderLinks(currentUrl, body, nesting, callback) {
  if(nesting === 0) {
    return process.nextTick(callback);
  }

  let links = utilities.getPageLinks(currentUrl, body); //[1]
  function iterate(index) { //[2]
    if(index === links.length) {
      return callback();
    }

    spider(links[index], nesting - 1, function(err) { //[3]
      if(err) {
        return callback(err);
      }
      iterate(index + 1);
    });
  }
  iterate(0); //[4]
}
```

从这个新功能中的重要步骤如下：

1. 我们使用`utilities.getPageLinks()`函数获取页面中包含的所有链接的列表。此函数仅返回指向相同主机名的链接。
2. 我们使用一个称为`iterate()`的本地函数来遍历链接，该函数需要下一个链接的索引进行分析。在这个函数中，我们首先要检查索引是否等于链接数组的长度，如果等于则是迭代完成，在这种情况下我们立即调用`callback()`函数，因为这意味着我们处理了所有的项目。
3. 这时，处理链接已准备就绪。我们通过递归调用`spider()`函数。
4. 作为`spiderLinks()`函数的最后一步也是最重要的一步，我们通过调用`iterate(0)`来开始迭代。


我们刚刚提出的算法允许我们通过顺序执行异步操作来迭代数组，在我们的例子中是`spider()`函数。

我们现在可以尝试这个新版本的`Web爬虫`应用程序，并观看它一个接一个地递归地下载网页的所有链接。要中断这个过程，如果有很多链接可能需要一段时间，请记住我们可以随时使用`Ctrl + C`。如果我们决定恢复它，我们可以通过启动`Web爬虫`应用程序并提供与上次结束时相同的`URL`来恢复执行。

现在我们的网络`Web爬虫`应用程序可能会触发整个网站的下载，请仔细考虑使用它。例如，不要设置高嵌套级别或离开爬虫运行超过几秒钟。用数千个请求重载服务器是不道德的。在某些情况下，这也被认为是非法的。需要考虑后果！

##### 迭代模式
我们之前展示的`spiderLinks()`函数的代码是一个清楚的例子，说明了如何在应用异步操作时迭代集合。我们还可以注意到，这是一种可以适应任何其他情况的模式，我们需要在集合的元素或通常的任务列表上按顺序异步迭代。该模式可以推广如下：

```javascript
function iterate(index) {
  if (index === tasks.length) {
    return finish();
  }
  const task = tasks[index];
  task(function() {
    iterate(index + 1);
  });
}

function finish() {
  // 迭代完成的操作
}

iterate(0);
```

注意到，如果`task()`是同步操作，这些类型的算法变得真正递归。在这种情况下，可能造成调用栈的溢出。

我们刚刚提出的模式是非常强大的，因为它可以适应几种情况。例如，我们可以映射数组的值，或者我们可以将迭代的结果传递给迭代中的下一个，以实现一个reduce算法，如果满足特定的条件，我们可以提前退出循环，或者甚至可以迭代无限数量的元素。

我们还可以选择将解决方案进一步推广：

```javascript
iterateSeries(collection, iteratorCallback, finalCallback);
```

通过创建一个名为`iterator`的函数来执行任务列表，该函数调用集合中的下一个可执行的任务，并确保在当前任务完成时调用迭代器结束的回调函数。

### 并行
在某些情况下，一组异步任务的执行顺序并不重要，我们只需要在所有这些运行的任务完成时通知我们。使用并行执行流更好地处理这种情况，如下图所示：

![](http://oczira72b.bkt.clouddn.com/17-10-14/18606236.jpg)

如果我们认为`Node.js`是单线程的话，这可能听起来很奇怪，但是如果我们记住我们在第一章中讨论过的内容，我们意识到即使我们只有一个线程，我们仍然可以实现并发，由于`Node.js`的非阻塞性质。实际上，在这种情况下，并行字不正确地使用，因为这并不意味着任务同时运行，而是它们的执行由底层的非阻塞`API`执行，并由事件循环进行交织。

我们知道，当一个任务允许事件循环执行另一个任务时，或者是说一个任务允许控制回到事件循环。这种工作流的名称为并发，但为了简单起见，我们仍然会使用并行。

下图显示了两个异步任务可以在`Node.js`程序中并行运行：

![](http://oczira72b.bkt.clouddn.com/17-10-14/11401894.jpg)

通过上图，我们有一个`Main`函数执行两个异步任务：

1. `Main`函数触发`Task 1`和`Task 2`的执行。由于这些触发异步操作，这两个函数会立即返回，并将控制权返还给主函数，之后等到事件循环完成再通知主线程。
2. 当`Task 1`的异步操作完成时，事件循环给与其线程控制权。当`Task 1`同步操作完成时，它通知`Main`函数。
3. 当`Task 2`的异步操作完成时，事件循环给与其线程控制权。当`Task 2`同步操作完成时，它再次通知`Main`函数。在这一点上，`Main`函数知晓`Task 1`和`Task 2`都已经执行完毕，所以它可以继续执行其后操作或将操作的结果返回给另一个回调函数。

简而言之，这意味着在`Node.js`中，我们只能执行并行异步操作，因为它们的并发性由非阻塞`API`在内部处理。在`Node.js`中，同步阻塞操作不能同时运行，除非它们的执行与异步操作交错，或者通过`setTimeout()`或`setImmediate()`延迟。我们将在第九章中更详细地看到这一点。

### Web爬虫版本3