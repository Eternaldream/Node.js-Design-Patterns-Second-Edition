# Node.js Essential Patterns
对于`Node.js`而言，异步特性是其最显著的特征，但对于别的一些语言，例如`PHP`，就不常处理异步代码。

在同步的编程中，我们习惯于把代码的执行想象为自上而下连续的执行计算步骤。每个操作都是阻塞的，这意味着只有在一个操作执行完成后才能执行下一个操作，这种方式利于我们理解和调试。

然而，在异步的编程中，我们可以在后台执行诸如读取文件或执行网络请求的一些操作。当我们在调用异步操作方法时，即使当前或之前的操作尚未完成，下面的后续操作也会继续执行，在后台执行的操作会在任意时刻执行完毕，并且应用程序会在异步调用完成时以正确的方式做出反应。

虽然这种非阻塞方法相比于阻塞方法性能更好，但它实在是让程序员难以理解，并且，在处理较为复杂的异步控制流的高级应用程序时，异步顺序可能会变得难以操作。

`Node.js`提供了一系列工具和设计模式，以便我们最佳地处理异步代码。了解如何使用它们编写性能和易于理解和调试的应用程序非常重要。

在本章中，我们将看到两个最重要的异步模式：回调和事件发布器。

## 回调模式
在上一章中介绍过，回调是`reactor模式`的`handler`的实例，回调本来就是`Node.js`独特的编程风格之一。回调函数是在异步操作完成后传播其操作结果的函数，总是用来替代同步操作的返回指令。而`JavaScript`恰好就是表示回调的最好的语言。在`JavaScript`中，函数是一等公民，我们可以把函数变量作为参数传递，并在另一个函数中调用它，把调用的结果存储到某一数据结构中。实现回调的另一个理想结构是闭包。使用闭包，我们能够保留函数创建时所在的上下文环境，这样，无论何时调用回调，都保持了请求异步操作的上下文。

在本节中，我们分析基于回调的编程思想和模式，而不是同步操作的返回指令的模式。

### [CPS](https://en.wikipedia.org/wiki/Continuation-passing_style)
在`JavaScript`中，回调函数作为参数传递给另一个函数，并在操作完成时调用。在函数式编程中，这种传递结果的方法被称为`CPS`。这是一个一般概念，而且不只是对于异步操作而言。实际上，它只是通过将结果作为参数传递给另一个函数（回调函数）来传递结果，然后在主体逻辑中调用回调函数拿到操作结果，而不是直接将其返回给调用者。

#### 同步CPS
为了更清晰地理解`CPS`，让我们来看看这个简单的同步函数：

```javascript
function add(a, b) {
  return a + b;
}
```

上面的例子成为直接编程风格，其实没什么特别的，就是使用`return`语句把结果直接传递给调用者。它代表的是同步编程中返回结果的最常见方法。上述功能的`CPS`写法如下：

```javascript
function add(a, b, callback) {
  callback(a + b);
}
```

`add()`函数是一个同步的`CPS`函数，`CPS`函数只会在它调用的时候才会拿到`add()`函数的执行结果，下列代码就是其调用方式：

```javascript
console.log('before');
add(1, 2, result => console.log('Result: ' + result));
console.log('after');
```

既然`add()`是同步的，那么上述代码会打印以下结果：

```
before
Result: 3
after
```

#### 异步CPS
那我们思考下面的这个例子，这里的`add()`函数是异步的：

```javascript
function additionAsync(a, b, callback) {
 setTimeout(() => callback(a + b), 100);
}
```

在上边的代码中，我们使用`setTimeout()`模拟异步回调函数的调用。现在，我们调用`additionalAsync`，并查看具体的输出结果。

```javascript
console.log('before');
additionAsync(1, 2, result => console.log('Result: ' + result));
console.log('after');
```

上述代码会有以下的输出结果：

```
before
after
Result: 3
```

因为`setTimeout()`是一个异步操作，所以它不会等待执行回调，而是立即返回，将控制权交给`addAsync()`，然后返回给其调用者。`Node.js`中的此属性至关重要，因为只要有异步请求产生，控制权就会交给事件循环，从而允许处理来自队列的新事件。

下面的图片显示了`Node.js`中事件循环过程：

![](http://oczira72b.bkt.clouddn.com/17-9-24/64301048.jpg)

当异步操作完成时，执行权就会交给这个异步操作开始的地方，即回调函数。执行将从**事件循环**开始，所以它将有一个新的堆栈。对于`JavaScript`而言，这是它的优势所在。正是由于闭包保存了其上下文环境，即使在不同的时间点和不同的位置调用回调，也能够正常地执行。

同步函数在其完成操作之前是阻塞的。而异步函数立即返回，结果将在事件循环的稍后循环中传递给处理程序（在我们的例子中是一个回调）。

#### 非CPS风格的回调模式
某些情况下情况下，我们可能会认为回调CPS式的写法像是异步的，然而并不是。比如以下代码，`Array`对象的`map()`方法：

```javascript
const result = [1, 5, 7].map(element => element - 1);
console.log(result); // [0, 4, 6]
```

在上述例子中，回调仅用于迭代数组的元素，而不是传递操作的结果。实际上，这个例子中是使用回调的方式同步返回，而非传递结果。是否是传递操作结果的回调通常在`API`文档有明确说明。

### 同步还是异步？

我们已经看到代码的执行顺序会因同步或异步的执行方式产生根本性的改变。这对整个应用程序的流程，正确性和效率都产生了重大影响。以下是对这两种模式及其缺陷的分析。一般来说，必须避免的是由于其执行顺序不一致导致的难以检测和拓展的混乱。下面是一个有陷阱的异步实例：

#### 一个有问题的函数
最危险的情况之一是在特定条件下同步执行本应异步执行的`API`。以下列代码为例：

```javascript
const fs = require('fs');
const cache = {};

function inconsistentRead(filename, callback) {
  if (cache[filename]) {
    // 如果缓存命中，则同步执行回调
    callback(cache[filename]);
  } else {
    // 未命中，则执行异步非阻塞的I/O操作
    fs.readFile(filename, 'utf8', (err, data) => {
      cache[filename] = data;
      callback(data);
    });
  }
}
```

上述功能使用缓存来存储不同文件读取操作的结果。不过记得，这只是一个例子，它缺少错误处理，并且其缓存逻辑本身不是最佳的（比如没有缓存淘汰策略）。除此之外，上述函数是非常危险的，因为如果没有设置高速缓存，它的行为是异步的，直到`fs.readFile()`函数返回结果为止，它都不会同步执行，这时缓存并不会触发，而会去走异步回调调用。

#### 解放[zalgo](https://github.com/oren/oren.github.io/blob/master/posts/zalgo.md)
关于`zalgo`，其实就是指同步或异步行为的不确定性，几乎总是导致非常难追踪的`bug`。

现在，我们来看看如何使用一个不可预测其顺序的函数，它甚至可以轻松地中断一个应用程序。看以下代码：

```javascript
function createFileReader(filename) {
  const listeners = [];
  inconsistentRead(filename, value => {
    listeners.forEach(listener => listener(value));
  });
  return {
    onDataReady: listener => listeners.push(listener)
  };
}
```

当上述函数被调用时，它创建一个充当事件发布器的新对象，允许我们为文件读取操作设置多个事件监听器。当读取操作完成并且数据可用时，所有的监听器将被立即被调用。前面的函数使用之前定义的`inconsistentRead()`函数来实现这个功能。我们现在尝试调用`createFileReader()`函数：

```javascript
const reader1 = createFileReader('data.txt');
reader1.onDataReady(data => {
 console.log('First call data: ' + data);
 // 之后再次通过fs读取同一个文件
 const reader2 = createFileReader('data.txt');
 reader2.onDataReady(data => {
   console.log('Second call data: ' + data);
 });
});
```

之后的输出是这样的：

![](http://oczira72b.bkt.clouddn.com/17-9-24/18060650.jpg)

```
First call data: some data
```

下面来分析为何第二次的回调没有被调用：

在创建`reader1`的时候，`inconsistentRead()`函数是异步执行的，这时没有可用的缓存结果，因此我们有时间注册事件监听器。在读操作完成后，它将在下一次事件循环中被调用。

然后，在事件循环的循环中创建`reader2`，其中所请求文件的缓存已经存在。在这种情况下，内部调用`inconsistentRead()`将是同步的。所以，它的回调将被立即调用，这意味着`reader2`的所有监听器也将被同步调用。然而，在创建`reader2`之后，我们才开始注册监听器，所以它们将永远不被调用。

`inconsistentRead()`回调函数的行为是不可预测的，因为它取决于许多因素，例如调用的频率，作为参数传递的文件名，以及加载文件所花费的时间等。

在实际应用中，例如我们刚刚看到的错误可能会非常复杂，难以在真实应用程序中识别和复制。想象一下，在`Web服务器`中使用类似的功能，可以有多个并发请求;想象一下这些请求挂起，没有任何明显的理由，没有任何日志被记录。这绝对属于烦人的`bug`。

`npm`的创始人和以前的`Node.js`项目负责人`Isaac Z. Schlueter`在他的一篇博客文章中比较了使用这种不可预测的功能来释放`Zalgo`。如果您不熟悉`Zalgo`。可以看看[Isaac Z. Schlueter的原始帖子](http://blog.izs.me/post/59142742143/designing-apis-for-asynchrony)。

#### 使用同步API
从上述关于`zalgo`的示例中，我们知道，`API`必须清楚地定义其性质：是同步的还是异步的？

我们合适`fix`上述的`inconsistentRead()`函数产生的`bug`的方式是使它完全同步阻塞执行。并且这是完全可能的，因为`Node.js`为大多数基本`I/O`操作提供了一组同步方式的`API`。例如，我们可以使用`fs.readFileSync()`函数来代替它的异步对等体。代码现在如下：

```javascript
const fs = require('fs');
const cache = {};

function consistentReadSync(filename) {
 if (cache[filename]) {
   return cache[filename];
 } else {
   cache[filename] = fs.readFileSync(filename, 'utf8');
   return cache[filename];
 }
}
```

我们可以看到整个函数被转化为同步阻塞调用的模式。如果一个函数是同步的，那么它不会是`CPS`的风格。事实上，我们可以说，使用`CPS`来实现一个同步的`API`一直是最佳实践，这将消除其性质上的任何混乱，并且从性能角度来看也将更加有效。

请记住，将`API`从`CPS`更改为直接调用返回的风格，或者说从异步到同步的风格。例如，在我们的例子中，我们必须完全改变我们的`createFileReader()`为同步，并使其适应于始终工作。


另外，使用同步`API`而不是异步`API`，要特别注意以下注意事项：

* 同步`API`并不适用于所有应用场景。
* 同步`API`将阻塞事件循环并将并发请求置于阻塞状态。它会破坏`JavaScript`的并发模型，甚至使得整个应用程序的性能下降。我们将在本书后面看到这对我们的应用程序的影响。

在我们的`inconsistentRead()`函数中，因为每个文件名仅调用一次，所以同步阻塞调用而对应用程序造成的影响并不大，并且缓存值将用于所有后续的调用。如果我们的静态文件的数量是有限的，那么使用`consistentReadSync()`将不会对我们的事件循环产生很大的影响。如果我们文件数量很大并且都需要被读取一次，而且对性能要求较高的情况下，我们不建议在`Node.js`中使用同步`I/O`。然而，在某些情况下，同步`I/O`可能是最简单和最有效的解决方案。所以我们必须正确评估具体的应用场景，以选择最为合适的方案。上述实例其实说明：在实际应用程序中使用同步阻塞`API`加载配置文件是非常有意义的。

因此，记得只有不影响应用程序并发能力时才考虑使用同步阻塞`I/O`。

#### 延时处理
另一种`fix`上述的`inconsistentRead()`函数产生的`bug`的方式是让它仅仅是异步的。这里的解决办法是下一次事件循环时同步调用，而不是在相同的事件循环周期中立即运行，使得其实际上是异步的。在`Node.js`中，可以使用`process.nextTick()`，它延迟函数的执行，直到下一次传递事件循环。它的功能非常简单，它将回调作为参数，并将其推送到事件队列的顶部，在任何未处理的`I/O`事件前，并立即返回。一旦事件循环再次运行，就会立刻调用回调。

所以看下列代码，我们可以较好的利用这项技术处理`inconsistentRead()`的异步顺序：

```javascript
const fs = require('fs');
const cache = {};

function consistentReadAsync(filename, callback) {
  if (cache[filename]) {
    // 下一次事件循环立即调用
    process.nextTick(() => callback(cache[filename]));
  } else {
    // 异步I/O操作
    fs.readFile(filename, 'utf8', (err, data) => {
      cache[filename] = data;
      callback(data);
    });
  }
}
```


现在，上述函数保证在任何情况下异步地调用其回调函数，解决了上述`bug`。

