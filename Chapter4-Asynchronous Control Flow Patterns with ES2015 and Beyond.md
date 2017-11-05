# Asynchronous Control Flow Patterns with ES2015 and Beyond
在上一章中，我们学习了如何使用回调处理异步代码，以及如何解决如回调地狱代码等异步问题。回调是`JavaScript`和`Node.js`中的异步编程的基础，但是现在，其他替代方案已经出现。这些替代方案更复杂，以便能够以更方便的方式处理异步代码。

在本章中，我们将探讨一些代表性的替代方案，`Promise`和`Generator`。以及`async await`，这是一种创新的语法，可在高版本的`JavaScript`中提供，其也作为`ECMAScript 2017`发行版的一部分。

我们将看到这些替代方案如何简化处理异步控制流的方式。最后，我们将比较所有这些方法，以了解所有这些方法的所有优点和缺点，并能够明智地选择最适合我们下一个`Node.js`项目要求的方法。

## Promise
我们在前面的章节中提到，`CPS风格`不是编写异步代码的唯一方法。事实上，`JavaScript`生态系统为传统的回调模式提供了有趣的替代方案。最着名的选择之一是`Promise`，特别是现在它是`ECMAScript 2015`的一部分，并且现在可以在`Node.js`中可用。

### 什么是Promise？
`Promise`是一种抽象的对象，我们通常允许函数返回一个名为`Promise`的对象，它表示异步操作的最终结果。通常情况下，我们说当异步操作尚未完成时，我们说`Promise`对象处于`pending`状态，当操作成功完成时，我们说`Promise`对象处于`resolve`状态，当操作错误终止时，我们说`Promise`对象处于`reject`状态。一旦`Promise`处于`resolve`或`reject`，我们认为当前异步操作结束。

为了接收到异步操作的正确结果或错误捕获，我们可以使用`Promise`的`then`方法：

```javascript
promise.then([onFulfilled], [onRejected])
```

在前面的代码中，`onFulfilled()`是一个函数，最终会收到`Promise`的正确结果，而`onRejected()`是另一个函数，它将接收产生异常的原因（如果有的话）。两个参数都是可选的。

要了解`Promise`如何转换我们的代码，让我们考虑以下几点：

```javascript
asyncOperation(arg, (err, result) => {
  if (err) {
    // 错误处理
  }
  // 正常结果处理
});
```

`Promise`允许我们将这个典型的`CPS`代码转换成更好的结构化和更优雅的代码，如下所示：

```javascript
asyncOperation(arg)
  .then(result => {
    // 错误处理
  }, err => {
    // 正常结果处理
  });
```

`then()`方法的一个关键特征是它同步地返回另一个`Promise`对象。如果`onFulfilled()`或`onRejected()`函数中的任何一个函数返回`x`，则`then()`方法返回的`Promise`对象将如下所示：

* 如果`x`是一个值，则这个`Promise`对象会正确处理(`resolve`)`x`
* 如果`x`是一个`Promise`对象或`thenable`，则会正确处理(`resolve`)`x`
* 如果`x`是一个异常，则会捕获异常(`reject`)`x`

> 注：thenable是一个具有then方法的类似于Promise的对象(Promise-like)。

这个特点使我们能够链式构建`Promise`，允许轻松排列组合我们的异步操作。另外，如果我们没有指定一个`onFulfilled()`或`onRejected()`处理程序，则正确结果或异常捕获将自动转发到`Promise`链的下一个`Promise`。例如，这允许我们在整个链中自动传播错误，直到被`onRejected()`处理程序捕获。随着`Promise`链，任务的顺序执行突然变成简单多了：

```javascript
asyncOperation(arg)
  .then(result1 => {
    // 返回另一个Promise
    return asyncOperation(arg2);
  })
  .then(result2 => {
    // 返回一个值
    return 'done';
  })
  .then(undefined, err => {
    // 捕获Promise链中的异常
  });
```

下图展示了链式`Promise`如何工作：

![](http://oczira72b.bkt.clouddn.com/17-10-31/46370382.jpg)

`Promise`的另一个重要特性是`onFulfilled()`和`onRejected()`函数是异步调用的，如同上述的例子，在最后那个`then`函数`resolve`一个同步的`Promise`，它也是同步的。这种模式避免了`Zalgo`（参见`Chapter2-Node.js Essential Patterns`），使我们的异步代码更加一致和稳健。

如果在`onFulfilled()`或`onRejected()`处理程序中抛出异常（使用`throw`语句），则`then()`方法返回的`Promise`将被自动地`reject`，抛出异常作为`reject`的原因。这相对于`CPS`来说是一个巨大的优势，因为它意味着有了`Promise`，异常将在整个链中自动传播，并且`throw`语句终于可以使用。

在以前，许多不同的库实现了`Promise`，大多数时候它们之间不兼容，这意味着不可能在使用不同`Promise`库的`thenable`链式传播错误。

`JavaScript`社区非常努力地解决了这个限制，这些努力导致了`Promises / A +`规范的创建。该规范详细描述了`then`方法的行为，提供了一个可互兼容的基础，这使得来自不同库的`Promise`对象能够彼此兼容，开箱即用。

有关`Promises / A +`规范的详细说明，可以参考[Promises / A + 官方网站](https://promisesaplus.com)。

### Promise / A + 的实施
在`JavaScript`中以及`Node.js`中，有几个实现`Promises / A +`规范的库。以下是最受欢迎的：

* [Bluebird](https://npmjs.org/package/bluebird)
* [Q](https://npmjs.org/package/q)
* [RSVP](https://npmjs.org/package/rsvp)
* [Vow](https://npmjs.org/package/vow)
* [When.js](https://npmjs.org/package/when)
* ES2015 promises

真正区别他们的是在`Promises / A +`标准之上提供的额外功能。正如我们上述所说的那样，该标准定义了`then()`方法和`Promise`解析过程的行为，但它没有指定其他功能，例如，如何从基于回调的异步函数创建`Promise`。

在我们的示例中，我们将使用由`ES2015`的`Promise`，因为`Promise`对象自`Node.js 4`后即可使用，而不需要任何库来实现。

作为参考，以下是`ES2015`的`Promise`提供的API：

`constructor`(`new Promise（function（resolve, reject）{}）`)：创建了一个新的`Promise`，它基于作为传递两个类型为函数的参数来决定`resolve`或`reject`。构造函数的参数解释如下：

* `resolve(obj)` ：`resolve`一个`Promise`，并带上一个参数`obj`，如果`obj`是一个值，这个值就是传递的异步操作成功的结果。如果`obj`是一个`Promise`或一个`thenable`，则会进行正确处理。
* `reject(err)`：`reject`一个`Promise`，并带上一个参数`err`。它是`Error`对象的一个实例。

#### Promise对象的静态方法
* `Promise.resolve(obj)`： 将会创建一个`resolve`的`Promise`实例
* `Promise.reject(err)`： 将会创建一个`reject`的`Promise`实例
* `Promise.all(iterable)`：返回一个新的`Promise`实例，并且在`iterable`中所
有`Promise`状态为`reject`时, 返回的`Promise`实例的状态会被置为`reject`，如果`iterable`中至少有一个`Promise`状态为`reject`时, 返回的`Promise`实例状态也会被置为`reject`，并且`reject`的原因是第一个被`reject`的`Promise`对象的`reject`原因。
* `Promise.race(iterable)`：返回一个`Promise`实例，当`iterable`中任何一个`Promise`被`resolve`或被`reject`时， 返回的`Promise`实例以同样的原因`resolve`或`reject`。

#### Promise实例方法
* `Promise.then(onFulfilled, onRejected)`：这是`Promise`的基本方法。它的行为与我们之前描述的`Promises / A +`标准兼容。
* `Promise.catch(onRejected)`：这只是`Promise.then(undefined，onRejected)`的语法糖。

> 值得一提的是，一些Promise实现提供了另一种机制来创建新的Promise，称为deferreds。我们不会在这里描述，因为它不是ES2015标准的一部分，但是如果您想了解更多信息，可以阅读Q文档 (https://github.com/kriskowal/q#using-deferreds) 或When.js文档 (https://github.com/cujojs/when/wiki/Deferred) 。

### Promisifying一个Node.js回调风格的函数
在`JavaScript`中，并不是所有的异步函数和库都支持开箱即用的`Promise`。大多数情况下，我们必须将一个典型的基于回调的函数转换成一个返回`Promise`的函数，这个过程也被称为`promisification`。

幸运的是，`Node.js`中使用的回调约定允许我们创建一个可重用的函数，我们通过使用`Promise`对象的构造函数来简化任何`Node.js`风格的`API`。让我们创建一个名为`promisify()`的新函数，并将其包含到`utilities.js`模块中（以便稍后在我们的`Web爬虫应用程序`中使用它）：

```javascript
module.exports.promisify = function(callbackBasedApi) {
  return function promisified() {
    const args = [].slice.call(arguments);
    return new Promise((resolve, reject) => {
      args.push((err, result) => {
        if (err) {
          return reject(err);
        }
        if (arguments.length <= 2) {
          resolve(result);
        } else {
          resolve([].slice.call(arguments, 1));
        }
      });
      callbackBasedApi.apply(null, args);
    });
  }
};
```

前面的函数返回另一个名为`promisified()`的函数，它表示输入中给出的`callbackBasedApi`的`promisified`版本。以下展示它是如何工作的：

1. `promisified()`函数使用`Promise`构造函数创建一个新的`Promise`对象，并立即将其返回给调用者。
2. 在传递给`Promise`构造函数的函数中，我们确保传递给`callbackBasedApi`，这是一个特殊的回调函数。由于我们知道回调总是最后调用的，我们只需将回调函数附加到提供给`promisified()`函数的参数列表里（`args`）。
3. 在特殊的回调中，如果我们收到错误，我们立即`reject`这个`Promise`。
4. 如果没有收到错误，我们使用一个值或一个数组值来`resolve`这个`Promise`，具体取决于传递给回调的结果数量。
5. 最后，我们只需使用我们构建的参数列表调用`callbackBasedApi`。

> 大部分的Promise已经提供了一个开箱即用的接口来将一个Node.js风格的API转换成一个返回Promise的API。例如，Q有Q.denodeify()和Q.nbind()，Bluebird有Promise.promisify()，而When.js有node.lift()。

### 顺序执行
在一些必要的理论之后，我们现在准备将我们的`Web爬虫应用程序`转换为使用`Promise`的形式。让我们直接从版本2开始，直接下载一个Web网页的链接。

在`spider.js`模块中，第一步是加载我们的`Promise`实现（我们稍后会使用它）和`Promisifying`我们打算使用的基于回调的函数：

```javascript
const utilities = require('./utilities');
const request = utilities.promisify(require('request'));
const mkdirp = utilities.promisify(require('mkdirp'));
const fs = require('fs');
const readFile = utilities.promisify(fs.readFile);
const writeFile = utilities.promisify(fs.writeFile);
```

现在，我们开始更改我们的`download`函数：

```javascript
function download(url, filename) {
  console.log(`Downloading ${url}`);
  let body;
  return request(url)
    .then(response => {
      body = response.body;
      return mkdirp(path.dirname(filename));
    })
    .then(() => writeFile(filename, body))
    .then(() => {
      console.log(`Downloaded and saved: ${url}`);
      return body;
    });
}
```

这里要注意的到的最重要的是我们也为`readFile()`返回的`Promise`注册
一个`onRejected()`函数，用来处理一个网页没有被下载的情况(或文件不存在)。 还有，看我们如何使用`throw`来传递`onRejected()`函数中的错误的。

既然我们已经更改我们的`spider()`函数，我们这么修改它的调用方式：

```javascript
spider(process.argv[2], 1)
  .then(() => console.log('Download complete'))
  .catch(err => console.log(err));
```

注意我们是如何第一次使用`Promise`的语法糖`catch`来处理源自`spider()`函数的任何错误情况。如果我们再看看迄今为止我们所写的所有代码，那么我们会惊喜的发现，我们没有包含任何错误传播逻辑，因为我们在使用回调函数时会被迫做这样的事情。这显然是一个巨大的优势，因为它极大地减少了我们代码中的样板文件以及丢失任何异步错误的机会。

现在，完成我们唯一缺失的`Web爬虫应用程序`的第二版的`spiderLinks()`函数，我们将在稍后实现它。

### 顺序迭代
到目前为止，`Web爬虫应用程序`代码库主要是对`Promise`是什么以及如何使用的概述，展示了使用`Promise`实现顺序执行流程的简单性和优雅性。但是，我们现在考虑的代码只涉及到一组已知的异步操作的执行。所以，完成我们对顺序执行流程的探索的缺失部分是看我们如何使用`Promise`来实现迭代。同样，网络蜘蛛第二版的`spiderLinks()`函数也是一个很好的例子。

让我们添加缺少的这一块：

```javascript
function spiderLinks(currentUrl, body, nesting) {
  let promise = Promise.resolve();
  if (nesting === 0) {
    return promise;
  }
  const links = utilities.getPageLinks(currentUrl, body);
  links.forEach(link => {
    promise = promise.then(() => spider(link, nesting - 1));
  });
  return promise;
}
```

为了异步迭代一个网页的全部链接，我们必须动态创建一个`Promise`的迭代链。

1. 首先，我们定义一个空的`Promise`，`resolve`为`undefined`。这个`Promise`只是用来作为`Promise`的迭代链的起始点。 
2. 然后，我们通过在循环中调用链中前一个`Promise`的`then()`方法获得的新的`Promise`来更新`Promise`变量。这就是我们使用`Promise`的异步迭代模式。

这样，循环的结束，`promise`变量会包含循环中最后一个`then()`返回的`Promise`对象，所以它只有当`Promise`的迭代链中全部`Promise`对象被`resolve`后才能被`resolve`。

> 注：在最后调用了这个then方法来resolve这个Promise对象

通过这个，我们已使用`Promise`对象重写了我们的`Web爬虫应用程序`。我们现在应该可以运行它了。

### 顺序迭代模式
为了总结这个顺序执行的部分，让我们提取一个模式来依次遍历一组`Promise`：

```javascript
let tasks = [ /* ... */ ]
let promise = Promise.resolve();
tasks.forEach(task => {
  promise = promise.then(() => {
    return task();
  });
});
promise.then(() => {
  // 所有任务都完成
});
```

使用`reduce()`方法来替代`forEach()`方法，允许我们写出更为简洁的代码：

```javascript
let tasks = [ /* ... */ ]
let promise = tasks.reduce((prev, task) => {
  return prev.then(() => {
    return task();
  });
}, Promise.resolve());

promise.then(() => {
  //All tasks completed
});
```

与往常一样，通过对这种模式的简单调整，我们可以将所有任务的结果收集到一个数组中，我们可以实现一个`mapping`算法，或者构建一个`filter`等等。

> 上述这个模式使用循环动态地建立一个链式的Promise。

### 并行执行
另一个适合用`Promise`的执行流程是并行执行流程。实际上，我们需要做的就是使用内置的`Promise.all()`。这个方法创造了另一个`Promise`对象，只有在输入中的所有`Promise`都`resolve`时才能`resolve`。这是一个并行执行，因为在其参数`Promise`对象的之间没有执行顺序可言。

为了演示这一点，我们来看我们的`Web爬虫应用程序`的第三版，它将页面中的所有链接并行下载。让我们再次使用`Promise`更新`spiderLinks()`函数来实现并行流程：

```javascript
function spiderLinks(currentUrl, body, nesting) {
  if (nesting === 0) {
    return Promise.resolve();
  }
  const links = utilities.getPageLinks(currentUrl, body);
  const promises = links.map(link => spider(link, nesting - 1));
  return Promise.all(promises);
}
```

这里的模式在`elements.map()`迭代中产生一个数组，存放所有异步任务，之后便于同时启动`spider()`任务。这一次，在循环中，我们不等待以前的下载完成，然后开始一个新的下载任务：所有的下载任务在一个循环中一个接一个地开始。之后，我们利用`Promise.all()`方法，它返回一个新的`Promise`对象，当数组中的所有`Promise`对象都被`resolve`时，这个`Promise`对象将被`resolve`。换句话说，所有的下载任务完成，这正是我们想要的。

### 限制并行执行
不幸的是，`ES2015`的`Promise API`并没有提供一种原生的方式来限制并发任务的数量，但是我们总是可以依靠我们所学到的有关用普通`JavaScript`来限制并发。事实上，我们在`TaskQueue`类中实现的模式可以很容易地被调整来支持返回承诺的任务。这很容易通过修改`next()`方法来完成：

```javascript
class TaskQueue {
  constructor(concurrency) {
    this.concurrency = concurrency;
    this.running = 0;
    this.queue = [];
  }

  pushTask(task) {
    this.queue.push(task);
    this.next();
  }

  next() {
    while (this.running < this.concurrency && this.queue.length) {
      const task = this.queue.shift();
      task().then(() => {
        this.running--;
        this.next();
      });
      this.running++;
    }
  }
}
```

不同于使用一个回调函数来处理任务，我们简单地调用`Promise`的`then()`。

让我们回到`spider.js`模块，并修改它以支持我们的新版本的`TaskQueue`类。首先，我们确保定义一个`TaskQueue`的新实例：

```javascript
const TaskQueue = require('./taskQueue');
const downloadQueue = new TaskQueue(2);
```

然后，是我们的`spiderLinks()`函数。这里的修改也是很简单：

```javascript
function spiderLinks(currentUrl, body, nesting) {
  if (nesting === 0) {
    return Promise.resolve();
  }
  const links = utilities.getPageLinks(currentUrl, body);
  // 我们需要如下代码，用于创建Promise对象
  // 如果没有下列代码，当任务数量为0时，将永远不会resolve
  if (links.length === 0) {
    return Promise.resolve();
  }
  return new Promise((resolve, reject) => {
    let completed = 0;
    let errored = false;
    links.forEach(link => {
      let task = () => {
        return spider(link, nesting - 1)
          .then(() => {
            if (++completed === links.length) {
              resolve();
            }
          })
          .catch(() => {
            if (!errored) {
              errored = true;
              reject();
            }
          });
      };
      downloadQueue.pushTask(task);
    });
  });
}
```

在上述代码中有几点值得我们注意的：

* 首先，我们需要返回使用`Promise`构造函数创建的新的`Promise`对象。正如我们将看到的，这使我们能够在队列中的所有任务完成时手动`resolve`我们的`Promise`对象。
* 然后，我们应该看看我们如何定义任务。我们所做的是将一个`onFulfilled()`回调函数的调用添加到由`spider()`返回的`Promise`对象中，所以我们可以计算完成的下载任务的数量。当完成的下载量与当前页面中链接的数量相同时，我们知道任务已经处理完毕，所以我们可以调用外部`Promise`的`resolve()`函数。

> Promises / A +规范规定，then()方法的onFulfilled()和onRejected()回调函数只能调用一次（仅调用onFulfilled()和onRejected()）。Promise接口的实现确保即使我们多次手动调用resolve或reject，Promise也仅可以被resolve或reject一次。

现在，使用`Promise`的`Web爬虫应用程序`的第4版应该已经准备好了。我们可能再次注意到下载任务如何并行运行，并发数量限制为2。

### 在公有API中暴露回调函数和Promise
正如我们在前面所学到的，`Promise`可以被用作回调函数的一个很好的替代品。它们使我们的代码更具可读性和易于理解。虽然`Promise`带来了许多优点，但也要求开发人员理解许多不易于理解的概念，以便正确和熟练地使用。由于这个原因和其他原因，在某些情况下，比起`Promise`来说，很多开发者更偏向于回调函数。

现在让我们想象一下，我们想要构建一个执行异步操作的公共库。我们需要做什么？我们是创建了一个基于回调函数的`API`还是一个面向`Promise`的`API`？还是两者均有？

这是许多知名的库所面临的问题，至少有两种方法值得一提，使我们能够提供一个多功能的`API`。

像`request`，`redis`和`mysql`这样的库所使用的第一种方法是提供一个简单的基于回调函数的`API`，如果需要，开发人员可以选择公开函数。其中一些库提供工具函数来`Promise`化异步回调，但开发人员仍然需要以某种方式将暴露的`API`转换为能够使用`Promise`对象。

第二种方法更透明。它还提供了一个面向回调的`API`，但它使回调参数可选。每当回调作为参数传递时，函数将正常运行，在完成时或失败时执行回调。当回调未被传递时，函数将立即返回一个`Promise`对象。这种方法有效地结合了回调函数和`Promise`，使得开发者可以在调用时选择采用什么接口，而不需要提前进行`Promise`化。许多库，如`mongoose`和`sequelize`，都支持这种方法。

我们来看一个简单的例子。假设我们要实现一个异步执行除法的模块：

```javascript
module.exports = function asyncDivision(dividend, divisor, cb) {
  return new Promise((resolve, reject) => { // [1]
    process.nextTick(() => {
      const result = dividend / divisor;
      if (isNaN(result) || !Number.isFinite(result)) {
        const error = new Error('Invalid operands');
        if (cb) {
          cb(error); // [2]
        }
        return reject(error);
      }
      if (cb) {
        cb(null, result); // [3]
      }
      resolve(result);
    });
  });
};
```

该模块的代码非常简单，但是有一些值得强调的细节：

* 首先，返回使用`Promise`的构造函数创建的新承诺。我们在构造函数参数函数内定义全部逻辑。
* 在发生错误的情况下，我们`reject`这个`Promise`，但如果回调函数在被调用时作为参数传递，我们也执行回调来进行错误传播。
* 在计算结果之后，我们`resolve`了这个`Promise`，但是如果有回调函数，我们也会将结果传播给回调函数。

我们现在看如何用回调函数和`Promise`来使用这个模块：

```javascript
// 回调函数的方式
asyncDivision(10, 2, (error, result) => {
  if (error) {
    return console.error(error);
  }
  console.log(result);
});

// Promise化的调用方式
asyncDivision(22, 11)
  .then(result => console.log(result))
  .catch(error => console.error(error));
```

应该很清楚的是，即将开始使用类似于上述的新模块的开发人员将很容易地选择最适合自己需求的风格，而无需在希望利用`Promise`时引入外部`promisification`功能。

## Generators
`ES2015`规范引入了另外一种机制，除了其他新功能外，还可以用来简化`Node.js`应用程序的异步控制流程。我们正在谈论`Generator`，也被称为`semi-coroutines`。它们是子程序的一般化，可以有不同的入口点。在一个正常的函数中，实际上我们只能有一个入口点，这个入口点对应着函数本身的调用。`Generator`与一般函数类似，但是可以暂停（使用`yield`语句），然后在稍后继续执行。在实现迭代器时，`Generator`特别有用，因为我们已经讨论了如何使用迭代器来实现重要的异步控制流模式，如顺序执行和限制并行执行。

### Generators基础
