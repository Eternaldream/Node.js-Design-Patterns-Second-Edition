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
在我们探索使用`Generator`来实现异步控制流程之前，学习一些基本概念是很重要的。我们从语法开始吧。可以通过在函数关键字之后附加`*`（星号）运算符来声明`Generator`函数：

```javascript
function* makeGenerator() {
  // body
}
```

在`makeGenerator()`函数内部，我们可以使用关键字`yield`暂停执行并返回给调用者传递给它的值：

```javascript
function* makeGenerator() {
  yield 'Hello World';
  console.log('Re-entered');
}
```

在前面的代码中，`Generator`通过`yield`一个字符串`Hello World`暂停当前函数的执行。当`Generator`恢复时，执行将从下列语句开始：

```javascript
console.log('Re-entered');
```

`makeGenerator()`函数本质上是一个工厂，它在被调用时返回一个新的`Generator`对象：

```javascript
const gen = makeGenerator();
```

生成器对象的最重要的方法是`next()`，它用于启动/恢复`Generator`的执行，并返回如下形式的对象：

```
{
  value: <yielded value>
  done: <true if the execution reached the end>
}
```

这个对象包含`Generator` `yield`的值和一个指示`Generator`是否已经完成执行的符号。

### 一个简单的例子
为了演示`Generator`，我们来创建一个名为`fruitGenerator.js`的新模块：

```javascript
function* fruitGenerator() {
  yield 'apple';
  yield 'orange';
  return 'watermelon';
}
const newFruitGenerator = fruitGenerator();
console.log(newFruitGenerator.next()); // [1]
console.log(newFruitGenerator.next()); // [2]
console.log(newFruitGenerator.next()); // [3]
```

前面的代码将打印下面的输出：

```
{ value: 'apple', done: false }
{ value: 'orange', done: false }
{ value: 'watermelon', done: true }
```

我们可以这么解释上述现象：

* 第一次调用`newFruitGenerator.next()`时，`Generator`函数开始执行，直到达到第一个`yield`语句为止，该命令暂停`Generator`函数执行，并将值`apple`返回给调用者。
* 在第二次调用`newFruitGenerator.next()`时，`Generator`函数恢复执行，从第二个`yield`语句开始，这又使得执行暂停，同时将`orange`返回给调用者。
* `newFruitGenerator.next()`的最后一次调用导致`Generator`函数的执行从其最后的`yield`恢复，一个返回语句，它终止`Generator`函数，返回`watermelon`，并将结果对象中的`done`属性设置为`true`。

### Generators作为迭代器
为了更好地理解为什么`Generator`函数对实现迭代器非常有用，我们来构建一个例子。在我们将调用`iteratorGenerator.js`的新模块中，我们编写下面的代码：

```javascript
function* iteratorGenerator(arr) {
  for (let i = 0; i < arr.length; i++) {
    yield arr[i];
  }
}
const iterator = iteratorGenerator(['apple', 'orange', 'watermelon']);
let currentItem = iterator.next();
while (!currentItem.done) {
  console.log(currentItem.value);
  currentItem = iterator.next();
}
```

此代码应按如下所示打印数组中的元素：

```
apple
orange
watermelon
```

在这个例子中，每次我们调用`iterator.next()`时，我们都会恢复`Generator`函数的`for`循环，通过`yield`数组中的下一个项来运行另一个循环。这演示了如何在函数调用过程中维护`Generator`的状态。当继续执行时，循环和所有变量的值与`Generator`函数执行暂停时的状态完全相同。

### 传值给Generators
现在我们继续研究`Generator`的基本功能，首先学习如何将值传递回`Generator`函数。这其实很简单，我们需要做的只是为`next()`方法提供一个参数，并且该值将作为`Generator`函数内的`yield`语句的返回值提供。

为了展示这一点，我们来创建一个新的简单模块：

```javascript
function* twoWayGenerator() {
  const what = yield null;
  console.log('Hello ' + what);
}
const twoWay = twoWayGenerator();
twoWay.next();
twoWay.next('world');
```

当执行时，前面的代码会输出`Hello world`。我们做如下的解释：

* 第一次调用`next()`方法时，`Generator`函数到达第一个`yield`语句，然后暂停。
* 当`next('world')`被调用时，`Generator`函数从上次停止的位置，也就是上次的`yield`语句点恢复，但是这次我们有一个值传递到`Generator`函数。这个值将被赋值到`what`变量。生成器然后执行`console.log()`指令并终止。

用类似的方式，我们可以强制`Generator`函数抛出异常。这可以通过使用`Generator`函数的`throw`方法来实现，如下例所示：

```javascript
const twoWay = twoWayGenerator();
twoWay.next();
twoWay.throw(new Error());
```

在这个最后这段代码，`twoWayGenerator()`函数将在`yield`函数返回的时候抛出异常。这就好像从`Generator`函数内部抛出了一个异常一样，这意味着它可以像使用`try ... catch`块一样进行捕获和处理异常。

### Generator实现异步控制流
你一定想知道`Generator`函数如何帮助我们处理异步操作。我们可以通过创建一个接受`Generator`函数作为参数的特殊函数来演示这一点，并允许我们在`Generator`函数内部使用异步代码。这个函数在异步操作完成时要注意恢复`Generator`函数的执行。我们将调用这个函数`asyncFlow()`：

```javascript
function asyncFlow(generatorFunction) {
  function callback(err) {
    if (err) {
      return generator.throw(err);
    }
    const results = [].slice.call(arguments, 1);
    generator.next(results.length > 1 ? results : results[0]);
  }
  const generator = generatorFunction(callback);
  generator.next();
}
```

前面的函数取一个`Generator`函数作为输入，然后立即调用：

```javascript
const generator = generatorFunction(callback);
generator.next();
```

`generatorFunction()`接受一个特殊的回调函数作为参数，当`generator.throw()`如果接收到一个错误，便立即返回。另外，通过将在回调函数中接收的`results`传值回`Generator`函数继续`Generator`函数的执行:

```javascript
if (err) {
  return generator.throw(err);
}
const results = [].slice.call(arguments, 1);
generator.next(results.length > 1 ? results : results[0]);
```

为了说明这个简单的辅助函数的强大，我们创建一个叫做`clone.js`的新模块，这个模块只是创建它本身的克隆。粘贴我们刚才创建的`asyncFlow()`函数，核心代码如下：

```javascript
const fs = require('fs');
const path = require('path');
asyncFlow(function*(callback) {
  const fileName = path.basename(__filename);
  const myself = yield fs.readFile(fileName, 'utf8', callback);
  yield fs.writeFile(`clone_of_${filename}`, myself, callback);
  console.log('Clone created');
});
```

明显地，有了`asyncFlow()`函数的帮助，我们可以像我们书写同步阻塞函数一样用同步的方式来书写异步代码了。并且这个结果背后的原理显得很清楚。一旦异步操作结束，传递给每个异步函数的回调函数将继续`Generator`函数的执行。没有什么复杂的，但是结果确实很令人意外。

这个技术有其他两个变化，一个是`Promise`的使用，另外一个则是`thunks`。

> 在基于Generator的控制流中使用的thunk只是一个简单的函数，它除了回调之外，部分地应用了原始函数的所有参数。返回值是另一个只接受回调作为参数的函数。例如，fs.readFile（）的thunkified版本如下所示：

```javascript
function readFileThunk(filename, options) {
  return function(callback) {
    fs.readFile(filename, options, callback);
  }
}
```

`thunk`和`Promise`都允许我们创建不需要回调的`Generator`函数作为参数传递，例如，使用`thunk`的`asyncFlow()`版本如下：

```javascript
function asyncFlowWithThunks(generatorFunction) {
  function callback(err) {
    if (err) {
      return generator.throw(err);
    }
    const results = [].slice.call(arguments, 1);
    const thunk = generator.next(results.length > 1 ? results : results[0]).value;
    thunk && thunk(callback);
  }
  const generator = generatorFunction();
  const thunk = generator.next().value;
  thunk && thunk(callback);
}
```

这个技巧是读取`generator.next()`的返回值，返回值中包含`thunk`。下一步是通过注入特殊的回调函数调用`thunk`本身。这允许我们写下面的代码：

```javascript
asyncFlowWithThunk(function*() {
  const fileName = path.basename(__filename);
  const myself = yield readFileThunk(__filename, 'utf8');
  yield writeFileThunk(`clone_of_${fileName}`, myself);
  console.log("Clone created")
});
```

### 使用co的基于Gernator的控制流
你应该已经猜到了，`Node.js`生态系统会借助`Generator`函数来提供一些处理异步控制流的解决方案，例如，[suspend](https://npm/js.org/package/suspend)是其中一个最老的支持`Promise`、`thunks`和`Node.js`风格回调函数和正常风格的回调函数的 库。还有，大部分我们之前分析的`Promise`库都提供工具函数使得`Generator`和`Promise`可以一起使用。

我们选择[co](https://npmjs.org/package/co)作为本章节的例子。它支持很多类型的`yieldables`，其中一些是：

* `Thunks`
* `Promises`
* `Arrays`(并行执行)
* `Objects`(并行执行)
* `Generators`(委托)
* `Generator`函数(委托)

还有很多框架或库是基于`co`生态系统的，包括以下一些：

* `Web框架`，最流行的是[koa](https://npmjs.org/package/koa)
* 实现特定控制流模式的库
* 包装流行的`API`兼容`co`的库

我们使用`co`重新实现我们的`Generator`版本的`Web爬虫应用程序`。

为了将`Node.js`风格的函数转换成`thunks`，我们将会使用一个叫做[thunkify](https://npmjs.org/package/thunkify)的库。

### 顺序执行
让我们通过修改`Web爬虫应用程序`的版本2开始我们对`Generator`函数和`co`的实际探索。我们要做的第一件事就是加载我们的依赖包，并生成我们要使用的函数的`thunkified`版本。这些将在`spider.js`模块的最开始进行：

```javascript
const thunkify = require('thunkify');
const co = require('co');
const request = thunkify(require('request'));
const fs = require('fs');
const mkdirp = thunkify(require('mkdirp'));
const readFile = thunkify(fs.readFile);
const writeFile = thunkify(fs.writeFile);
const nextTick = thunkify(process.nextTick);
```

看上述代码，我们可以注意到与本章前面`promisify`化的`API`的代码的一些相似之处。在这一点上，有意思的是，如果我们使用我们的`promisified`版本的函数来代替`thunkified`的版本，代码将保持完全一样，这要归功于`co`支持`thunk`和`Promise`对象作为`yieldable`对象。事实上，如果我们想，甚至可以在同一个应用程序中使用`thunk`和`Promise`，即使在同一个`Generator`函数中。就灵活性而言，这是一个巨大的优势，因为它使我们能够使用基于`Generator`函数的控制流来解决我们应用程序中的问题。

好的，现在让我们开始将`download()`函数转换为一个`Generator`函数：

```javascript
function* download(url, filename) {
  console.log(`Downloading ${url}`);
  const response = yield request(url);
  const body = response[1];
  yield mkdirp(path.dirname(filename));
  yield writeFile(filename, body);
  console.log(`Downloaded and saved ${url}`);
  return body;
}
```

通过使用`Generator`和`co`，我们的`download()`函数变得简单多了。当我们需要做异步操作的时候，我们使用异步的`Generator`函数作为`thunk`来把之前的内容转化到`Generator`函数，并使用`yield`子句。

然后我们开始实现我们的`spider()`函数：

```javascript
function* spider(url, nesting) {
  cost filename = utilities.urlToFilename(url);
  let body;
  try {
    body = yield readFile(filename, 'utf8');
  } catch (err) {
    if (err.code !== 'ENOENT') {
      throw err;
    }
    body = yield download(url, filename);
  }
  yield spiderLinks(url, body, nesting);
}
```

从上述代码中一个有趣的细节是我们可以使用`try...catch`语句块来处理异常。我们还可以使用`throw`来传播异常。另外一个细节是我们`yield`我们的`download()`函数，而这个函数既不是一个`thunk`，也不是一个`promisified`函数，只是另外的一个`Generator`函数。这也毫无问题，由于`co`也支持其他`Generators`作为`yieldables`。

最后转换`spiderLinks()`，在这个函数中，我们递归下载一个网页的链接。在这个函数中使用`Generators`，显得简单多了：

```javascript
function* spiderLinks(currentUrl, body, nesting) {
  if (nesting === 0) {
    return nextTick();
  }
  const links = utilities.getPageLinks(currentUrl, body);
  for (let i = 0; i < links.length; i++) {
    yield spider(links[i], nesting - 1);
  }
}
```

看上述代码。虽然顺序迭代没有什么模式可以展示。`Generator`和`co`辅助我们做了很多，方便了我们可以使用同步方式开书写异步代码。

看最重要的部分，程序的入口：

```javascript
co(function*() {
  try {
    yield spider(process.argv[2], 1);
    console.log(`Download complete`);
  } catch (err) {
    console.log(err);
  }
});
```

这是唯一一处需要调用`co(...)`来封装的一个`Generator`。实际上，一旦我们这么做，`co`会自动封装我们传递给`yield`语句的任何`Generator`函数，并且这个过程是递归的，所以程序的剩余部分与我们是否使用`co`是完全无关的，虽然是被`co`封装在里面。

现在应该可以运行使用`Generator`函数改写的`Web爬虫应用程序`了。

### 并行执行
不幸的是，虽然`Generator`很方便地进行顺序执行，但是不能直接用来并行化执行一组任务，至少不能仅仅使用`yield`和`Generator`。之前，在种情况下我们使用的模式只是简单地依赖于一个基于回调或者`Promise`的函数，但使用了`Generator`函数后，一切会显得更简单。

幸运的是，如果不限制并发数的并行执行，`co`已经可以通过`yield`一个`Promise`对象、`thunk`、`Generator`函数，甚至包含`Generator`函数的数组来实现。

考虑到这一点，我们的`Web爬虫应用程序`第三版可以通过重写`spiderLinks()`函数来做如下改动：

```javascript
function* spiderLinks(currentUrl, body, nesting) {
  if (nesting === 0) {
    return nextTick();
  }
  const links = utilities.getPageLinks(currentUrl, body);
  const tasks = links.map(link => spider(link, nesting - 1));
  yield tasks;
}
```

但是上述函数所做的只是拿到所有的任务，这些任务本质上都是通过`Generator`函数来实现异步的，如果在`co`的`thunk`内对一个包含`Generator`函数的数组使用`yield`，这些任务都会并行执行。外层的`Generator`函数会等到`yield`子句的所有异步任务并行执行后再继续执行。

接下来我们看怎么用一个基于回调函数的方式来解决相同的并行流。我们用这种方式重写`spiderLinks()`函数：

```javascript
function spiderLinks(currentUrl, body, nesting) {
  if (nesting === 0) {
    return nextTick();
  }
  // 返回一个thunk
  return callback => {
    let completed = 0,
      hasErrors = false;
    const links = utilities.getPageLinks(currentUrl, body);
    if (links.length === 0) {
      return process.nextTick(callback);
    }

    function done(err, result) {
      if (err && !hasErrors) {
        hasErrors = true;
        return callback(err);
      }
      if (++completed === links.length && !hasErrors) {
        callback();
      }
    }
    for (let i = 0; i < links.length; i++) {
      co(spider(links[i], nesting - 1)).then(done);
    }
  }
}
```

我们使用`co`并行运行`spider()`函数，调用`Generator`函数返回了一个`Promise`对象。这样，等待`Promise`完成后调用`done()`函数。通常，基于`Generator`控制流的库都有这一功能，因此如果需要，你总是可以将一个`Generator`转换成一个基于回调或基于`Promise`的函数。

为了并行开启多个下载任务，我们只要重用在前面定义的基于回调的并行执行的模式。我们应该也注意到我们将`spiderLinks()`转换成一个`thunk`(而不再是一个`Generator`函数)。这使得当全部并行任务完成时，我们有一个回调函数可以调用。

> 上面讲到的是将一个Generator函数转换为一个thunk的模式，使之能够支持其他的基于回调或基于Promise的控制流算法，并可以通过同步阻塞的代码风格书写异步代码。

### 限制并行执行
现在我们知道如何处理异步执行流程，应该很容易规划我们的`Web爬虫应用程序`的第四版的实现，这个版本对并发下载任务的数量施加了限制。我们有几个方案可以用来做到这一点。其中一些方案如下：

* 使用先前实现的基于回调的`TaskQueue`类。我们只需要`thunkify`我们的`Generator`函数和其提供的回调函数即可。
* 使用基于`Promise`的`TaskQueue`类，并确保每个作为任务的`Generator`函数都被转换成一个返回`Promise`对象的函数。
* 使用`async`，`thunkify`我们打算使用的工具函数，此外还需要把我们用到的`Generator`函数转化为基于回调的模式，以便于能够被这个库较好地使用。
* 使用基于`co`的生态系统中的库，特别是专门为这种场景的库，如[co-limiter](https://npmjs.org/package/co-limiter)。
* 实现基于生产者 - 消费者模型的自定义算法，这与`co-limiter`的内部实现原理相同。

为了学习，我们选择最后一个方案，甚至帮助我们可以更好地理解一种经常与协程(也和线程和进程)同步相关的模式。

### 生产者 - 消费者模式
我们的目标是利用队列来提供固定数量的`workers`，与我们想要设置的并发级别一样多。为了实现这个算法，我们将基于本章前面定义的`TaskQueue`类改写：

```javascript
class TaskQueue {
  constructor(concurrency) {
    this.concurrency = concurrency;
    this.running = 0;
    this.taskQueue = [];
    this.consumerQueue = [];
    this.spawnWorkers(concurrency);
  }
  pushTask(task) {
    if (this.consumerQueue.length !== 0) {
      this.consumerQueue.shift()(null, task);
    } else {
      this.taskQueue.push(task);
    }
  }
  spawnWorkers(concurrency) {
    const self = this;
    for (let i = 0; i < concurrency; i++) {
      co(function*() {
        while (true) {
          const task = yield self.nextTask();
          yield task;
        }
      });
    }
  }
  nextTask() {
    return callback => {
      if (this.taskQueue.length !== 0) {
        return callback(null, this.taskQueue.shift());
      }
      this.consumerQueue.push(callback);
    }
  }
}
```

让我们分析这个`TaskQueue`类的新实现。首先是在构造函数中。需要调用一次`this.spawnWorkers()`，因为这是启动`worker`的方法。

我们的`worker`很简单，它们只是用`co()`包装的立即执行的`Generator`函数，所以每个`Generator`函数可以并行执行。在内部，每个`worker`正在运行在一个死循环（`while(true){}`）中，一直阻塞（`yield`）到新任务在队列中可用时（`yield self.nextTask()`），一旦可以执行新任务，`yield`这个异步任务直到其完成。您可能想知道我们如何能够限制并行执行，并让下一个任务在队列中处于等待状态。答案是在`nextTask()`方法中。我们来详细地看看在这个方法的原理：

```javascript
nextTask() {
  return callback => {
    if (this.taskQueue.length !== 0) {
      return callback(null, this.taskQueue.shift());
    }
    this.consumerQueue.push(callback);
  }
}
```

我们看这个函数内部发生了什么，这才是这个模式的核心：

1. 这个方法返回一个对于`co`而言是一个合法的`yieldable`的`thunk`。
2. 只要`taskQueue`类生成的实例中还有下一个任务，`thunk`的回调函数会被立即调用。回调函数调用时，立马解锁一个`worker`的阻塞状态，`yield`这一个任务。
3. 如果队列中没有任务了，回调函数本身会被放入`consumerQueue`中。通过这种做法，我们将一个`worker`置于空闲（`idle`）的模式。一旦我们有一个新的任务来要处理，在`consumerQueue`队列中的回调函数会被调用，立马唤醒我们这一`worker`进行异步处理。

现在，为了理解`consumerQueue`队列中的空闲`worker`是如何恢复工作的，我们需要分析`pushTask()`方法。如果当前有回调函数可用的话，`pushTask()`方法将调用`consumerQueue`队列中的第一个回调函数，从而将取消对`worker`的锁定。如果没有可用的回调函数，这意味着所有的`worker`都是工作状态，只需要添加一个新的任务到`taskQueue`任务队列中。

在`TaskQueue`类中，`worker`充当消费者的角色，而调用`pushTask()`函数的角色可以被认为是生产者。这个模式向我们展示了一个`Generator`函数实际上可以跟一个线程或进程类似。实际上，生产者 - 消费者之间问题是研究进程间通信和同步时最常见的问题，但正如我们已经提到的那样，它对于进程和线程来说，也是一个常见的例子。

### 限制下载任务的并发量
既然我们已经使用`Generator`函数和生产者 - 消费者模型实现一个限制并行算法，并且已经在`Web爬虫应用程序`第四版应用它来限制中下载任务的并发数。 首先，我们加载和初始化一个`TaskQueue`对象：

```javascript
const TaskQueue = require('./taskQueue');
const downloadQueue = new TaskQueue(2);
```

然后，修改`spiderLinks()`函数。和之前不限制并发的版本类似，所以这里我们只展示修改的部分，主要是通过调用新版本的`TaskQueue`类生成的实例的`pushTask()`方法来限制并行执行：

```javascript
function spiderLinks(currentUrl, body, nesting) {
  //...
  return (callback) => {
    //...
    function done(err, result) {
      //...
    }
    links.forEach(function(link) {
      downloadQueue.pushTask(function*() {
        yield spider(link, nesting - 1);
        done();
      });
    });
  }
}
```

在每个任务中，我们在下载完成后立即调用`done()`函数，因此我们可以计算下载了多少个链接，然后在完成下载时通知`thunk`的回调函数执行。

## 配合Babel使用Async await新语法
回调函数、`Promise`和`Generator`函数都是用于处理`JavaScript`和`Node.js`异步问题的方式。正如我们所看到的，`Generator`的真正意义在于它提供了一种方式来暂停一个函数的执行，然后等待前面的任务完成后再继续执行。我们可以使用这样的特性来书写异步代码，并且让开发者用同步阻塞的代码风格来书写异步代码。等到异步操作的结果返回后才恢复当前函数的执行。

但`Generator`函数是更多的是用来处理迭代器，然而迭代器在异步代码的使用显得有点笨重。代码可能难以理解，导致代码易读性和可维护性差。

但在不远的将来会有一种更加简洁的语法。实际上，这个提议即将引入到`ESMASCript 2017`的规范中，这项规范定义了`async`函数语法。

`async`函数规范引入两个关键字(`async`和`await`)到原生的`JavaScript`语言中，改进我们书写异步代码的方式。

为了理解这项语法的用法和优势为，我们看一个简单的例子：

```javascript
const request = require('request');

function getPageHtml(url) {
  return new Promise(function(resolve, reject) {
    request(url, function(error, response, body) {
      resolve(body);
    });
  });
}
async function main() {
  const html = await getPageHtml('http://google.com');
  console.log(html);
}

main();
console.log('Loading...');
```

在上述代码中，有两个函数：`getPageHtml`和`main`。第一个函数的作用是提取给定`URL`的一个远程网页的`HTML`文档代码。值得注意的是，这个函数返回一个`Promise`对象。

重点在于`main`函数，因为在这里使用了`async`和`await`关键字。首先要注意的是函数要以`async`关键字为前缀。意思是这个函数执行的是异步代码并且允许它在函数体内使用`await`关键字。`await`关键字在`getPageHtml`调用之前，告诉`JavaScript`解释器在继续执行下一条指令之前，等待`getPageHtml`返回的`Promise`对象的结果。这样，`main`函数内部哪部分代码是异步的，它会等待异步代码的完成再继续执行后续操作，并且不会阻塞这段程序其余部分的正常执行。实际上，控制台会打印字符串`Loading...`，随后是Google主页的`HTML`代码。

是不是这种方法的可读性更好并且更容易理解呢? 不幸地是，这个提议尚未定案，即使通过这个提议，我们需要等下一个版本
的`ECMAScript`规范出来并把它集成到`Node.js`后，才能使用这个新语法。 所以我们今天做了什么?只是漫无目的地等待?不是，当然不是！我们已经可以在我们的代码中使用`async await`语法，只要我们使用`Babel`。

### 安装与运行Babel
`Babel`是一个`JavaScript`编译器(或翻译器)，能够使用语法转换器将高版本的`JavaScript`代码转换成其他`JavaScript`代码。语法转换器允许例如我们书写并使用`ES2015`，`ES2016`，`JSX`和其它的新语法，来翻译成往后兼容的代码，在`JavaScript`运行环境如浏览器或`Node.js`中都可以使用`Babel`。

在项目中使用`npm`安装`Babel`，命令如下：

```
npm install --save-dev babel-cli
```

我们还需要安装插件以支持`async await`语法的解释或翻译：

```
npm install --save-dev babel-plugin-syntax-async-functions babel-plugin-tranform-async-to-generator
```

现在假设我们想运行我们之前的例子（称为`index.js`）。我们需要通过以下命令启动：

```
node_modules/.bin/babel-node --plugins "syntax-async-functions,transform-async-to-generator" index.js
```

这样，我们使用支持`async await`的转换器动态地转换源代码。`Node.js`运行的实际是保存在内存中的往后兼容的代码。

`Babel`也能被配置为一个代码构建工具，保存翻译或解释后的代码到本地文件系统中，便于我们部署和运行生成的代码。

> 关于如何安装和配置Babel，可以到官方网站 https://babeljs.io 查阅相关文档。

## 几种方式的比较
现在，我们应该对于怎么处理`JavaScript`的异步问题有了一个更好的认识和总结。在下面的表格中总结几大机制的优势和劣势：

![](http://oczira72b.bkt.clouddn.com/17-11-7/55293366.jpg)

> 值得一提的是，我们选择在本章中仅介绍处理异步控制流程的最受欢迎的解决方案，或者是广泛使用的解决方案，但是例如Fibers（ https://npmjs.org/package/fibers ）和Streamline（ https://npmjs.org/p ackage/streamline ）也是值得一看的。

## 总结
在本章中，我们分析了一些处理异步控制流的方法，分析了`Promise`、`Generator`函数和即将到来的`async await`语法。

我们学习了如何使用这些方法编写更简洁，更具有可读性的异步代码。我们讨论了这些方法的一些最重要的优点和缺点，并认识到即使它们非常有用，也需要一些时间来掌握。这就是这几种方式也没有完全取代在许多情况下仍然非常有用的回调的原因。作为一名开发人员，应该按照实际情况分析决定使用哪种解决方案。如果您正在构建执行异步操作的公共库，则应该提供易于使用的`API`，即使对于只想使用回调的开发人员也是如此。

在下一章中，我们将探讨另一个与异步代码执行相关的机制，这也是整个`Node.js`生态系统中的另一个基本构建块：`streams`。