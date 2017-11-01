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

> 值得一提的是，一些Promise实现提供了另一种机制来创建新的Promise，称为deferreds。我们不会在这里描述，因为它不是ES2015标准的一部分，但是如果您想了解更多信息，可以阅读Q文档(https://github.com/kriskowal/q#using-deferreds)或When.js(https://github.com/cujojs/when/wiki/Deferred)。
