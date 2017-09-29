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

另一个用于延迟执行代码的`API`是`setImmediate()`。虽然它们的作用看起来非常相似，但实际含义却截然不同。`process.nextTick()`的回调函数会在任何其他`I/O`操作之前调用，而对于`setImmediate()`则会在其它`I/O`操作之后调用。由于`process.nextTick()`在其它的`I/O`之前调用，因此在某些情况下可能会导致`I/O`进入无限期等待，例如递归调用`process.nextTick()`但是对于`setImmediate()`则不会发生这种情况。当我们在本书后面分析使用延迟调用来运行同步`CPU`绑定任务时，我们将深入了解这两种API之间的区别。


我们保证通过使用`process.nextTick()`异步调用其回调函数。

### Node.js回调风格
对于`Node.js`而言，`CPS`风格的`API`和回调函数遵循一组特殊的约定。这些约定不只是适用于`Node.js`核心`API`，对于它们之后也是绝大多数用户级模块和应用程序也很有意义。因此，我们了解这些风格，并确保我们在需要设计异步`API`时遵守规定显得至关重要。

#### 回调总是最后一个参数
在所有核心`Node.js`方法中，标准约定是当函数在输入中接受回调时，必须作为最后一个参数传递。我们以下面的`Node.js`核心`API`为例：

```javascript
fs.readFile(filename, [options], callback);
```

从前面的例子可以看出，即使是在可选参数存在的情况下，回调也始终置于最后的位置。其原因是在回调定义的情况下，函数调用更可读。

#### 错误处理总在最前
在`CPS`中，错误以不同于正确结果的形式在回调函数中传递。在`Node.js`中，`CPS`风格的回调函数产生的任何错误总是作为回调的第一个参数传递，并且任何实际的结果从第二个参数开始传递。如果操作成功，没有错误，第一个参数将为`null`或`undefined`。看下列代码：

```javascript
fs.readFile('foo.txt', 'utf8', (err, data) => {
  if (err)
    handleError(err);
  else
    processData(data);
});
```

上面的例子是最好的检测错误的方法，如果不检测错误，我们可能难以发现和调试代码中的`bug`，但另外一个要考虑的问题是错误总是为`Error`类型，这意味着简单的字符串或数字不应该作为错误对象传递（难以被`try catch`代码块捕获）。

#### 错误传播
对于同步阻塞的写法而言，我们的错误都是通过`throw`语句抛出，即使错误在错误栈中跳转，我们也能很好地捕获到错误上下文。

但是对于`CPS`风格的异步调用而言，通过把错误传递到错误栈中的下一个回调来完成，下面是一个典型的例子：

```javascript
const fs = require('fs');

function readJSON(filename, callback) {
  fs.readFile(filename, 'utf8', (err, data) => {
    let parsed;
    if (err)
    // 如果有错误产生则退出当前调用
      return callback(err);
    try {
      // 解析文件中的数据
      parsed = JSON.parse(data);
    } catch (err) {
      // 捕获解析中的错误，如果有错误产生，则进行错误处理
      return callback(err);
    }
    // 没有错误，调用回调
    callback(null, parsed);
  });
};
```

从上面的例子中我们注意到的细节是当我们想要正确地进行异常处理时，我们如何向`callback`传递参数。此外，当有错误产生时，我们使用了`return`语句，立即退出当前函数调用，避免进行下面的相关执行。

#### 不可捕获的异常
从上述`readJSON()`函数，为了避免将任何异常抛到`fs.readFile()`的回调函数中捕获，我们对`JSON.parse()`周围放置一个`try catch`代码块。在异步回调中一旦出错，将抛出异常，并跳转到事件循环，不把错误传播到下一个回调函数去。

在`Node.js`中，这是一个不可恢复的状态，应用程序会关闭，并将错误打印到标准输出中。为了证明这一点，我们尝试从之前定义的`readJSON()`函数中删除`try catch`代码块：

```javascript
const fs = require('fs');

function readJSONThrows(filename, callback) {
  fs.readFile(filename, 'utf8', (err, data) => {
    if (err) {
      return callback(err);
    }
    // 假设parse的执行没有错误
    callback(null, JSON.parse(data));
  });
};
```

在上面的代码中，我们没有办法捕获到`JSON.parse`产生的异常，如果我们尝试传递一个非标准`JSON`格式的文件，将会抛出以下错误：

![](http://oczira72b.bkt.clouddn.com/17-9-24/69634492.jpg)

```
SyntaxError: Unexpected token d
at Object.parse (native)
at [...]
at fs.js:266:14
at Object.oncomplete (fs.js:107:15)
```

现在，如果我们看看前面的错误栈跟踪，我们将看到它从`fs`模块的某处开始，恰好从本地`API`完成文件读取返回到`fs.readFile()`函数，通过事件循环。这些信息都很清楚地显示给我们，异常从我们的回调传入堆栈，然后直接进入事件循环，最终被捕获并抛出到控制台中。
这也意味着使用`try catch`代码块包装对`readJSONThrows()`的调用将不起作用，因为块所在的堆栈与调用回调的堆栈不同。以下代码显示了我们刚才描述的相反的情况：

```javascript
try {
  readJSONThrows('nonJSON.txt', function(err, result) {
    // ... 
  });
} catch (err) {
  console.log('This will not catch the JSON parsing exception');
}
```

前面的`catch`语句将永远不会收到`JSON`解析异常，因为它将返回到抛出异常的堆栈。我们刚刚看到堆栈在事件循环中结束，而不是触发异步操作的功能。
如前所述，应用程序在异常到达事件循环的那一刻中止，然而，我们仍然有机会在应用程序终止之前执行一些清理或日志记录。事实上，当这种情况发生时，`Node.js`会在退出进程之前发出一个名为`uncaughtException`的特殊事件。以下代码显示了一个示例用例：

```javascript

process.on('uncaughtException', (err) => {
  console.error('This will catch at last the ' +
    'JSON parsing exception: ' + err.message);
  // Terminates the application with 1 (error) as exit code:
  // without the following line, the application would continue
  process.exit(1);
});
```

![](http://oczira72b.bkt.clouddn.com/17-9-24/37979265.jpg)

重要的是，未被捕获的异常会使应用程序处于不能保证一致的状态，这可能导致不可预见的问题。例如，可能还有不完整的`I/O`请求运行或关闭可能会变得不一致。这就是为什么总是建议，特别是在生产环境中，在接收到未被捕获的异常之后写上述代码进行错误日志记录。

## 模块系统及相关模式
模块不仅是构建大型应用的基础，其主要机制是封装内部实现、方法与变量，通过接口。在本节中，我们将介绍`Node.js`的模块系统及其最常见的使用模式。
### 关于模块
`JavaScript`的主要问题之一是没有命名空间。在全局范围内运行的程序会污染全局命名空间，造成相关变量、数据、方法名的冲突。解决这个问题的技术称为模块模式，看下列代码：

```javascript
const module = (() => {
  const privateFoo = () => {
    // ...
  };
  const privateBar = [];
  const exported = {
    publicFoo: () => {
      // ...
    },
    publicBar: () => {
      // ...
    }
  };
  return exported;
})();
console.log(module);
```

此模式利用自执行匿名函数实现模块，仅导出旨希望被公开调用的部分。在上面的代码中，模块变量只包含导出的`API`，而其余的模块内容实际上从外部访问不到。我们将在稍后看到，这种模式背后的想法被用作`Node.js`模块系统的基础。

### Node.js模块相关解释
`CommonJS`是一个旨在规范`JavaScript`生态系统的组织，他们提出了`CommonJS模块规范`。 `Node.js`在此规范之上构建了其模块系统，并添加了一些自定义的扩展。为了描述它的工作原理，我们可以通过这样一个例子解释模块模式，每个模块都在私有命名空间下运行，这样模块内定义的每个变量都不会污染全局命名空间。

#### 自定义模块系统
为了解释模块系统的远离，让我们从头开始构建一个类似的模块系统。下面的代码创建一个模仿`Node.js`原始`require()`函数的功能。

我们先创建一个加载模块内容的函数，将其包装到一个私有的命名空间内：

```javascript
function loadModule(filename, module, require) {
  const wrappedSrc = `(function(module, exports, require) {
         ${fs.readFileSync(filename, 'utf8')}
       })(module, module.exports, require);`;
  eval(wrappedSrc);
}
```

模块的源代码被包装到一个函数中，如同自执行匿名函数那样。这里的区别在于，我们将一些固有的变量传递给模块，特指`module`，`exports`和`require`。注意导出模块的参数是`module.exports`和`exports`，后面我们将再讨论。

请记住，这只是一个例子，在真实项目中可不要这么做。诸如`eval()`或[vm模块](http://nodejs.org/api/vm.html)有可能导致一些安全性的问题，它人可能利用漏洞来进行注入攻击。我们应该非常小心地使用甚至完全避免使用`eval`。

我们现在来看模块的接口、变量等是如何被`require()`函数引入的：

```javascript
const require = (moduleName) => {
  console.log(`Require invoked for module: ${moduleName}`);
  const id = require.resolve(moduleName);
  // 是否命中缓存
  if (require.cache[id]) {
    return require.cache[id].exports;
  }
  // 定义module
  const module = {
    exports: {},
    id: id
  };
  // 新模块引入，存入缓存
  require.cache[id] = module;
  // 加载模块
  loadModule(id, module, require);
  // 返回导出的变量
  return module.exports;
};
require.cache = {};
require.resolve = (moduleName) => {
  /* 通过模块名作为参数resolve一个完整的模块 */
};
```

上面的函数模拟了用于加载模块的原生`Node.js`的`require()`函数的行为。当然，这只是一个`demo`，它并不能准确且完整地反映`require()`函数的真实行为，但是为了更好地理解`Node.js`模块系统的内部实现，定义模块和加载模块。我们的自制模块系统的功能如下：


* 模块名称被作为参数传入，我们首先做的是找寻模块的完整路径，我们称之为`id`。`require.resolve()`专门负责这项功能，它通过一个特定的解析算法实现相关功能（稍后将讨论）。
* 如果模块已经被加载，它应该存在于缓存。在这种情况下，我们立即返回缓存中的模块。
* 如果模块尚未加载，我们将首次加载该模块。创建一个模块对象，其中包含一个使用空对象字面值初始化的`exports`属性。该属性将被模块的代码用于导出该模块的公共`API`。
* 缓存首次加载的模块对象。
* 模块源代码从其文件中读取，代码被导入，如前所述。我们通过`require()`函数向模块提供我们刚刚创建的模块对象。该模块通过操作或替换`module.exports`对象来导出其公共API。
* 最后，将代表模块的公共`API`的`module.exports`的内容返回给调用者。

正如我们所看到的，`Node.js`模块系统的原理并不是想象中那么高深，只不过是通过我们一系列操作来创建和导入导出模块源代码。

#### 定义一个模块
通过查看我们的自定义`require()`函数的工作原理，我们现在既然已经知道如何定义一个模块。再来看下面这个例子：

```javascript
// 加载另一个模块
const dependency = require('./anotherModule');
// 模块内的私有函数
function log() {
  console.log(`Well done ${dependency.username}`);
}
// 通过导出API实现共有方法
module.exports.run = () => {
  log();
};
```


需要注意的是模块内的所有内容都是私有的，除非它被分配给`module.exports`变量。然后，当使用`require()`加载模块时，缓存并返回此变量的内容。

#### 定义全局变量
即使在模块中声明的所有变量和函数都在其本地范围内定义，仍然可以定义全局变量。事实上，模块系统公开了一个名为`global`的特殊变量。分配给此变量的所有内容将会被定义到全局环境下。

注意：污染全局命名空间是不好的，并且没有充分运用模块系统的优势。所以，只有真的需要使用全局变量，才去使用它。

#### module.exports和exports
对于许多还不熟悉`Node.js`的开发人员而言，他们最容易混淆的是`exports`和`module.exports`来导出公共`API`的区别。变量`export`只是对`module.exports`的初始值的引用;我们已经看到，`exports`本质上在模块加载之前只是一个简单的对象。

这意味着我们只能将新属性附加到导出变量引用的对象，如以下代码所示：

```javascript
exports.hello = () => {
  console.log('Hello');
}
```

重新给`exports`赋值并不会有任何影响，因为它并不会因此而改变`module.exports`的内容，它只是改变了该变量本身。因此下列代码是错误的：

```javascript
exports = () => {
  console.log('Hello');
}
```

如果我们想要导出除对象之外的内容，比如函数，我们可以给`module.exports`重新赋值：

```javascript
module.exports = () => {
  console.log('Hello');
}
```

#### require函数是同步的
另一个重要的细节是上述我们写的`require()`函数是同步的，它使用了一个较为简单的方式返回了模块内容，并且不需要回调函数。因此，对于`module.exports`也是同步的，例如，下列的代码是不正确的：

```javascript
setTimeout(() => {
  module.exports = function() {
    // ...
  };
}, 100);
```

通过这种方式导出模块会对我们定义模块产生重要的影响，因为它限制了我们同步定义并使用模块的方式。这实际上是为什么核心`Node.js`库提供同步`API`以代替异步`API`的最重要的原因之一。

如果我们需要定义一个需要异步操作来进行初始化的模块，我们也可以随时定义和导出需要我们异步初始化的模块。但是这样定义异步模块我们并不能保证`require()`后可以立即使用，在第九章，我们将详细分析这个问题，并提出一些模式来优化解决这个问题。

实际上，在早期的`Node.js`中，曾经有一个异步版本的`require()`，但由于它对初始化时间和异步`I/O`的性能有巨大影响，很快这个`API`就被删除了。

#### resolve算法
`依赖地狱`描述了软件的依赖于不同版本的软件包的依赖关系，`Node.js`通过加载不同版本的模块来解决这个问题，具体取决于模块的加载位置。而都是由`npm`来完成的，相关算法被称作`resolve算法`，被用到`require()`函数中。


现在让我们快速概述一下这个算法。如下所述，`resolve()`函数将一个模块名称（`moduleName`）作为输入，并返回模块的完整路径。然后，该路径用于加载其代码，并且还可以唯一地标识模块。`resolve算法`可以分为以下三种规则：

* 文件模块：如果`moduleName`以`/`开头，那么它已经被认为是模块的绝对路径。如果以`./`开头，那么`moduleName`被认为是相对路径，它是从使用`require`的模块的位置开始计算的。
* 核心模块：如果`moduleName`不以`/`或`./`开头，则算法将首先尝试在核心Node.js模块中进行搜索。
* 模块包：如果没有找到匹配`moduleName`的核心模块，则搜索在当前目录下的`node_modules`，如果没有搜索到`node_modules`，则会往上层目录继续搜索`node_modules`，直到它到达文件系统的根目录。

对于文件和包模块，单个文件和目录也可以匹配到`moduleName`。特别地，算法将尝试匹配以下内容：
* `<moduleName>.js`
* `<moduleName>/index.js`
* 在`<moduleName>/package.json`的`main`值下声明的文件或目录

[resolve算法的具体文档](https://nodejs.org/api/modules.html#modules_all_together)

`node_modules`目录实际上是`npm`安装每个包并存放相关依赖关系的地方。这意味着，基于我们刚刚描述的算法，每个包都有自身的私有依赖关系。例如，看以下目录结构：

```

myApp
├── foo.js
└── node_modules
    ├── depA
    │   └── index.js
    └── depB
        │
        ├── bar.js
        ├── node_modules
        ├── depA
        │    └── index.js
        └── depC
             ├── foobar.js
             └── node_modules
                 └── depA
                     └── index.js
```


在前面的例子中，`myApp`，`depB`和`depC`都依赖于`depA`;然而，他们都有自己的私有依赖的版本！按照解析算法的规则，使用`require('depA')`将根据需要的模块加载不同的文件，如下：

* 在`/myApp/foo.js`中调用的`require('depA')`会加载`/myApp/node_modules/depA/index.js`
* 在`/myApp/node_modules/depB/bar.js`中调用的`require('depA')`会加载`/myApp/node_modules/depB/node_modules/depA/index.js`
* 在`/myApp/node_modules/depC/foobar.js`中调用的`require('depA')`会加载`/myApp/node_modules/depC/node_modules/depA/index.js`

`resolve算法`是`Node.js`依赖关系管理的核心部分，它的存在使得即便应用程序拥有成百上千包的情况下也不会出现冲突和版本不兼容的问题。

当我们调用`require()`时，解析算法对我们是透明的。然而，仍然可以通过调用`require.resolve()`直接由任何模块使用。

#### 模块缓存
每个模块只会在它第一次引入的时候加载，此后的任意一次`require()`调用均从之前缓存的版本中取得。通过查看我们之前写的自定义的`require()`函数，可以看到缓存对于性能提升至关重要，此外也具有一些其它的优势，如下：

* 使得模块依赖关系的重复利用成为可能
* 从某种程度上保证了在从给定的包中要求相同的模块时总是返回相同的实例，避免了冲突

模块缓存通过`require.cache`变量查看，因此如果需要，可以直接访问它。在实际运用中的例子是通过删除`require.cache`变量中的相对键来使某个缓存的模块无效，这是在测试过程中非常有用，但在正常情况下会十分危险。

#### 循环依赖
许多人认为循环依赖是`Node.js`内在的设计问题，但在真实项目中真的可能发生，所以我们至少知道如何在`Node.js`中使得循环依赖有效。再来看我们自定义的`require()`函数，我们可以立即看到其工作原理和注意事项。

看下面这两个模块：

* 模块`a.js`

```javascript
exports.loaded = false;
const b = require('./b');
module.exports = {
  bWasLoaded: b.loaded,
  loaded: true
};
```

* 模块`b.js`

```javascript
exports.loaded = false;
const a = require('./a');
module.exports = {
  aWasLoaded: a.loaded,
  loaded: true
};
```

然后我们在`main.js`中写以下代码：

```javascript
const a = require('./a');
const b = require('./b');
console.log(a);
console.log(b);
```

执行上述代码，会打印以下结果：

```javascript
{
  bWasLoaded: true,
  loaded: true
}
{
  aWasLoaded: false,
  loaded: true
}
```

![](http://oczira72b.bkt.clouddn.com/17-9-28/51159001.jpg)

这个结果展现了循环依赖的处理顺序。虽然`a.js`和`b.js`这两个模块都在主模块需要的时候完全初始化，但是当从`b.js`加载时，`a.js`模块是不完整的。特别，这种状态会持续到`b.js`加载完毕的那一刻。这种情况我们应该引起注意，特别要确认我们在`main.js`中两个模块所需的顺序。

这是由于模块`a.js`将收到一个不完整的版本的`b.js`。我们现在明白，如果我们失去了首先加载哪个模块的控制，如果项目足够大，这可能会很容易发生循环依赖。

[关于循环引用的文档](https://nodejs.org/api/modules.html#modules_cycles)

简单说就是，为了防止模块载入的死循环，`Node.js`在模块第一次载入后会把它的结果进行缓存，下一次再对它进行载入的时候会直接从缓存中取出结果。所以在这种循环依赖情形下，不会有死循环，但是却会因为缓存造成模块没有按照我们预想的那样被导出（`export`，详细的案例分析见下文）。

官网给出了三个模块还不是循环依赖最简单的情形。实际上，两个模块就可以很清楚的表达出这种情况。根据递归的思想，解决了最简单的情形，这一类任意大小规模的问题也就解决了一半（另一半还需要探明随着问题规模增长，问题的解将会如何变化）。

`JavaScript`作为一门解释型的语言，上面的打印输出清晰的展示出了程序运行的轨迹。在这个例子中，`a.js`首先`require`了`b.js`, 程序进入`b.js`，在`b.js`中第一行又`require`了`a.js`。

如前文所述，为了避免无限循环的模块依赖，在`Node.js`运行`a.js` 之后，它就被缓存了，但需要注意的是，此时缓存的仅仅是一个未完工的`a.js`（**an unfinished copy of the a.js**）。所以在 `b.js`中`require`了`a.js`时，得到的仅仅是缓存中一个未完工的`a.js`，具体来说，它并没有明确被导出的具体内容（`a.js`尾端）。所以`b.js`中输出的`a`是一个空对象。

之后，`b.js`顺利执行完，回到`a.js`的`require`语句之后，继续执行完成。

### 模块定义模式
模块系统除了自带处理依赖关系的机制之外，最常见的功能就是定义`API`。对于定义`API`，主要需要考虑私有和公共功能之间的平衡。其目的是最大化信息隐藏内部实现和暴露的`API`可用性，同时将这些与可扩展性和代码重用性进行平衡。

在本节中，我们将分析一些在`Node.js`中定义模块的最流行模式;每个模块都保证了私有变量的透明，可扩展性和代码重用。

#### 命名导出
暴露公共`API`的最基本方法是使用命名导出，其中包括将我们想要公开的所有值分配给由`export`（或`module.exports`）引用的对象的属性。以这种方式，生成的导出对象将成为一组相关功能的容器或命名空间。

看下面代码，是此模式的实现：

```javascript
//file logger.js
exports.info = (message) => {
  console.log('info: ' + message);
};
exports.verbose = (message) => {
  console.log('verbose: ' + message);
};
```

导出的函数随后作为引入其的模块的属性使用，如下面的代码所示：

```javascript
// file main.js
const logger = require('./logger');
logger.info('This is an informational message');
logger.verbose('This is a verbose message');
```

大多数`Node.js`模块使用这种定义。


`CommonJS`规范仅允许使用`exports`变量来公开`public`成员。因此，命名的导出模式是唯一与`CommonJS`规范兼容的模式。使用`module.exports`是`Node.js`提供的一个扩展，以支持更广泛的模块定义模式。

#### 函数导出
最流行的模块定义模式之一包括将整个`module.exports`变量重新分配给一个函数。它的主要优点是它只暴露了一个函数，为模块提供了一个明确的入口点，使其更易于理解和使用，它也很好地展现了单一职责原则。这种定义模块的方法在社区中也被称为`substack模式`，在以下示例中查看此模式：

```javascript
// file logger.js
module.exports = (message) => {
  console.log(`info: ${message}`);
};
```

该模式也可以将导出的函数用作其他公共`API`的命名空间。这是一个非常强大的组合，因为它仍然给模块一个单独的入口点（`exports`的主函数）。这种方法还允许我们公开具有次要或更高级用例的其他函数。以下代码显示了如何使用导出的函数作为命名空间来扩展我们之前定义的模块：

```javascript
module.exports.verbose = (message) => {
  console.log(`verbose: ${message}`);
};
```

这段代码演示了如何调用我们刚才定义的模块：

```javascript
// file main.js
const logger = require('./logger');
logger('This is an informational message');
logger.verbose('This is a verbose message');
```

虽然只是导出一个函数也可能是一个限制，但实际上它是一个完美的方式，把重点放在一个单一的函数，它代表着这个模块最重要的一个功能，同时使得内部私有变量属性更加透明，而只是暴露导出函数本身的属性。

`Node.js`的模块化鼓励我们遵循采用单一职责原则（`SRP`）：每个模块应该对单个功能负责，该职责应完全由该模块封装，以保证复用性。

注意，这里讲的`substack模式`，就是通过仅导出一个函数来暴露模块的主要功能。使用导出的函数作为命名空间来导出别的次要功能。

#### 构造器(类)导出
导出构造函数的模块是导出函数的模块的特例。其不同之处在于，使用这种新模式，我们允许用户使用构造函数创建新的实例，但是我们也可以扩展其原型并创建新类（继承）。以下是此模式的示例：

```javascript
// file logger.js
function Logger(name) {
  this.name = name;
}
Logger.prototype.log = function(message) {
  console.log(`[${this.name}] ${message}`);
};
Logger.prototype.info = function(message) {
  this.log(`info: ${message}`);
};
Logger.prototype.verbose = function(message) {
  this.log(`verbose: ${message}`);
};
module.exports = Logger;
```

我们通过以下方式使用上述模块：

```javascript
// file main.js
const Logger = require('./logger');
const dbLogger = new Logger('DB');
dbLogger.info('This is an informational message');
const accessLogger = new Logger('ACCESS');
accessLogger.verbose('This is a verbose message');
```

通过`ES2015`的`class`关键字语法也可以实现相同的模式：

```javascript
class Logger {
  constructor(name) {
    this.name = name;
  }
  log(message) {
    console.log(`[${this.name}] ${message}`);
  }
  info(message) {
    this.log(`info: ${message}`);
  }
  verbose(message) {
    this.log(`verbose: ${message}`);
  }
}
module.exports = Logger;
```

鉴于`ES2015`的类只是原型的语法糖，该模块的使用将与其基于原型和构造函数的方案完全相同。


导出构造函数或类仍然是模块的单个入口点，但与`substack模式`比起来，它暴露了更多的模块内部结构。然而，另一方面，当想要扩展该模块功能时，我们可以更加方便。

这种模式的变种包括对不使用`new`的调用。这个小技巧让我们将我们的模块用作工厂。看下列代码：

```javascript
function Logger(name) {
  if (!(this instanceof Logger)) {
    return new Logger(name);
  }
  this.name = name;
};
```

其实这很简单：我们检查`this`是否存在，并且是`Logger`的一个实例。如果这些条件中的任何一个都为`false`，则意味着`Logger()`函数在不使用`new`的情况下被调用，然后继续正确创建新实例并将其返回给调用者。这种技术允许我们将模块也用作工厂：

```javascript
// file logger.js
const Logger = require('./logger');
const dbLogger = Logger('DB');
accessLogger.verbose('This is a verbose message');
```

`ES2015`的`new.target`语法从`Node.js 6`开始提供了一个更简洁的实现上述功能的方法。该利用公开了`new.target`属性，该属性是所有函数中可用的`元属性`，如果使用`new`关键字调用函数，则在运行时计算结果为`true`。
我们可以使用这种语法重写工厂：

```javascript
function Logger(name) {
  if (!new.target) {
    return new LoggerConstructor(name);
  }
  this.name = name;
}
```

这个代码完全与前一段代码作用相同，所以我们可以说`ES2015`的`new.target`语法糖使得代码更加可读和自然。

#### 实例导出
我们可以利用`require()`的缓存机制来轻松地定义具有从构造函数或工厂创建的状态的有状态实例，可以在不同模块之间共享。以下代码显示了此模式的示例：

```javascript
//file logger.js
function Logger(name) {
  this.count = 0;
  this.name = name;
}
Logger.prototype.log = function(message) {
  this.count++;
  console.log('[' + this.name + '] ' + message);
};
module.exports = new Logger('DEFAULT');
```

这个新定义的模块可以这么使用：

```javascript
// file main.js
const logger = require('./logger');
logger.log('This is an informational message');
```

因为模块被缓存，所以每个需要`Logger`模块的模块实际上总是会检索该对象的相同实例，从而共享它的状态。这种模式非常像创建单例。然而，它并不保证整个应用程序的实例的唯一性，因为它发生在传统的单例模式中。在分析解析算法时，实际上已经看到，一个模块可能会多次安装在应用程序的依赖关系树中。这导致了同一逻辑模块的多个实例，所有这些实例都运行在同一个`Node.js`应用程序的上下文中。在第7章中，我们将分析导出有状态的实例和一些可替代的模式。

我们刚刚描述的模式的扩展包括`exports`用于创建实例的构造函数以及实例本身。这允许用户创建相同对象的新实例，或者如果需要也可以扩展它们。为了实现这一点，我们只需要为实例分配一个新的属性，如下面的代码所示：

```javascript
module.exports.Logger = Logger;
```

然后，我们可以使用导出的构造函数创建类的其他实例：

```javascript
const customLogger = new logger.Logger('CUSTOM');
customLogger.log('This is an informational message');
```

从代码可用性的角度来看，这类似于将导出的函数用作命名空间，该模块导出一个对象的默认实例，这是我们大部分时间使用的功能，而更多的高级功能（如创建新实例或扩展对象的功能）仍然可以通过较少的暴露属性来使用。

#### 修改其他模块或全局作用域

一个模块甚至可以导出任何东西这可以看起来有点不合适;但是，我们不应该忘记一个模块可以修改全局范围和其中的任何对象，包括缓存中的其他模块。请注意，这些通常被认为是不好的做法，但是由于这种模式在某些情况下（例如测试）可能是有用和安全的，有时确实可以利用这一特性，这是值得了解和理解的。我们说一个模块可以修改全局范围内的其他模块或对象。它通常是指在运行时修改现有对象以更改或扩展其行为或应用的临时更改。

以下示例显示了我们如何向另一个模块添加新函数：

```javascript
// file patcher.js
// ./logger is another module
require('./logger').customMessage = () => console.log('This is a new functionality');
```

编写以下代码：

```javascript
// file main.js
require('./patcher');
const logger = require('./logger');
logger.customMessage();
```

在上述代码中，必须首先引入`patcher`程序才能使用`logger`模块。

上面的写法是很危险的。主要考虑的是拥有修改全局命名空间或其他模块的模块是具有副作用的操作。换句话说，它会影响其范围之外的实体的状态，这可能导致不可预测的后果，特别是当多个模块与相同的实体进行交互时。想象一下，有两个不同的模块尝试设置相同的全局变量，或者修改同一个模块的相同属性，效果可能是不可预测的（哪个模块胜出？），但最重要的是它会对在整个应用程序产生影响。

## 观察者模式
`Node.js`中的另一个重要和基本的模式是观察者模式。与`reactor模式`，回调模式和模块一样，观察者模式是`Node.js`基础之一，也是使用许多`Node.js`核心模块和用户定义模块的基础。

观察者模式是对`Node.js`的数据响应的理想解决方案，也是对回调的完美补充。我们给出以下定义：

发布者定义一个对象，它可以在其状态发生变化时通知一组观察者（或监听者）。

与回调模式的主要区别在于，主体实际上可以通知多个观察者，而传统的`CPS`风格的回调通常主体的结果只会传播给一个监听器。

### EventEmitter类
在传统的面向对象编程中，观察者模式需要接口，具体类和层次结构。在`Node.js`中，都变得简单得多。观察者模式已经内置在核心模块中，可以通过`EventEmitter`类来实现。 `EventEmitter`类允许我们注册一个或多个函数作为监听器，当特定的事件类型被触发时，它的回调将被调用，以通知其监听器。以下图像直观地解释了这个概念：

![](http://oczira72b.bkt.clouddn.com/17-9-28/75441136.jpg)

`EventEmitter`是一个类（原型），它是从事件核心模块导出的。以下代码显示了如何获得对它的引用：

```javascript
const EventEmitter = require('events').EventEmitter;
const eeInstance = new EventEmitter();
```

`EventEmitter`的基本方法如下：
* `on(event，listener)`：此方法允许您为给定的事件类型（`String类型`）注册一个新的侦听器（一个函数）
* `once(event, listener)`：此方法注册一个新的监听器，然后在事件首次发布之后被删除
* `emit(event, [arg1], [...])`：此方法会生成一个新事件，并提供其他参数以传递给侦听器
* `removeListener(event, listener)`：此方法将删除指定事件类型的侦听器

所有上述方法将返回`EventEmitter`实例以允许链接。监听器函数`function([arg1], [...])`，所以它只是接受事件发出时提供的参数。在侦听器中，这是指`EventEmitter`生成事件的实例。
我们可以看到，一个监听器和一个传统的`Node.js`回调有很大的区别;特别地，第一个参数不是`error`，它是在调用时传递给`emit()`的任何数据。

### 创建和使用EventEmitter
我们来看看我们如何在实践中使用`EventEmitter`。最简单的方法是创建一个新的实例并立即使用它。以下代码显示了在文件列表中找到匹配特定正则的文件内容时，使用`EventEmitter`实现实时通知订阅者的功能：

```javascript
const EventEmitter = require('events').EventEmitter;
const fs = require('fs');

function findPattern(files, regex) {
  const emitter = new EventEmitter();
  files.forEach(function(file) {
    fs.readFile(file, 'utf8', (err, content) => {
      if (err)
        return emitter.emit('error', err);
      emitter.emit('fileread', file);
      let match;
      if (match = content.match(regex))
        match.forEach(elem => emitter.emit('found', file, elem));
    });
  });
  return emitter;
}
```

由前面的函数`EventEmitter`处理将产生的三个事件：

* `fileread`事件：当文件被读取时触发
* `found`事件：当文件内容被正则匹配成功时触发
* `error`事件：当读取文件出现错误时触发

下面看`findPattern()`函数是如何被触发的：

```javascript
findPattern(['fileA.txt', 'fileB.json'], /hello \w+/g)
  .on('fileread', file => console.log(file + ' was read'))
  .on('found', (file, match) => console.log('Matched "' + match + '" in file ' + file))
  .on('error', err => console.log('Error emitted: ' + err.message));
```


在前面的例子中，我们为`EventParttern()`函数创建的`EventEmitter`生成的每个事件类型注册了一个监听器。

### 错误传播

如果事件是异步发送的，`EventEmitter`不能在异常情况发生时抛出异常，异常会在事件循环中丢失。相反，而是`emit`是发出一个称为错误的特殊事件，`Error对象`通过参数传递。这正是我们在之前定义的`findPattern()`函数中正在做的。

对于错误事件，始终是最佳做法注册侦听器，因为`Node.js`会以特殊的方式处理它，并且如果没有找到相关联的侦听器，将自动抛出异常并退出程序。

### 让任意对象可观察
有时，直接通过`EventEmitter`类创建一个新的可观察的对象是不够的，因为原生`EventEmitter`类并没有提供我们实际运用场景的拓展功能。我们可以通过扩展`EventEmitter`类使一个通用对象可观察。

为了演示这个模式，我们试着在对象中实现`findPattern()`函数的功能，如下代码所示：

```javascript
const EventEmitter = require('events').EventEmitter;
const fs = require('fs');
class FindPattern extends EventEmitter {
  constructor(regex) {
    super();
    this.regex = regex;
    this.files = [];
  }
  addFile(file) {
    this.files.push(file);
    return this;
  }
  find() {
    this.files.forEach(file => {
      fs.readFile(file, 'utf8', (err, content) => {
        if (err) {
          return this.emit('error', err);
        }
        this.emit('fileread', file);
        let match = null;
        if (match = content.match(this.regex)) {
          match.forEach(elem => this.emit('found', file, elem));
        }
      });
    });
    return this;
  }
}
```

我们定义的`FindPattern`类中运用了核心模块`util`提供的`inherits()`函数来扩展`EventEmitter`。以这种方式，它成为一个符合我们实际运用场景的可观察类。以下是其用法的示例：

```javascript
const findPatternObject = new FindPattern(/hello \w+/);
findPatternObject
  .addFile('fileA.txt')
  .addFile('fileB.json')
  .find()
  .on('found', (file, match) => console.log(`Matched "${match}"
       in file ${file}`))
  .on('error', err => console.log(`Error emitted ${err.message}`));
```

现在，通过继承`EventEmitter`的功能，我们现在可以看到`FindPattern`对象除了可观察外，还有一整套方法。
这在`Node.js`生态系统中是一个很常见的模式，例如，核心`HTTP`模块的`Server`对象定义了`listen()`，`close()`，`setTimeout()`等方法，并且在内部它也继承自`EventEmitter`函数，从而允许它在收到新的请求、建立新的连接或者服务器关闭响应请求相关的事件。

扩展`EventEmitter`的对象的其他示例是`Node.js`流。我们将在第五章中更详细地分析`Node.js`的流。

### 同步和异步事件
与回调模式类似，事件也支持同步或异步发送。至关重要的是，我们决不应当在同一个`EventEmitter`中混合使用两种方法，但是在发布相同的事件类型时考虑同步或者异步显得至关重要，以避免产生因同步与异步顺序不一致导致的`zalgo`。

发布同步和异步事件的主要区别在于观察者注册的方式。当事件异步发布时，即使在`EventEmitter`初始化之后，程序也会注册新的观察者，因为必须保证此事件在事件循环下一周期之前不被触发。正如上边的`findPattern()`函数中的情况。它代表了大多数`Node.js`异步模块中使用的常用方法。

相反，同步发布事件要求在`EventEmitter`函数开始发出任何事件之前就得注册好观察者。看下面的例子：

```javascript
const EventEmitter = require('events').EventEmitter;
class SyncEmit extends EventEmitter {
  constructor() {
    super();
    this.emit('ready');
  }
}
const syncEmit = new SyncEmit();
syncEmit.on('ready', () => console.log('Object is ready to be  used'));
```

如果`ready`事件是异步发布的，那么上述代码将会正常运行，然而，由于事件是同步发布的，并且监听器在发送事件之后才被注册，所以结果不调用监听器，该代码将无法打印到控制台。

由于不同的应用场景，有时以同步方式使用`EventEmitter`函数是有意义的。因此，要清楚地突出我们的`EventEmitter`的同步和异步性，以避免产生不必要的错误和异常。

### 事件机制与回调机制的比较
在定义异步`API`时，常见的难点是检查是否使用`EventEmitter`的事件机制或仅接受回调函数。一般区分规则是这样的：当一个结果必须以异步方式返回时，应该使用回调函数，当需要结果不确定其方式时，应该使用事件机制来响应。

但是，由于这两者实在太相近，并且可能两种方式都能实现相同的应用场景，所以产生了许多混乱。以下列代码为例：

```javascript
function helloEvents() {
  const eventEmitter = new EventEmitter();
  setTimeout(() => eventEmitter.emit('hello', 'hello world'), 100);
  return eventEmitter;
}

function helloCallback(callback) {
  setTimeout(() => callback('hello world'), 100);
}
```

`helloEvents()`和`helloCallback()`在其功能上可以被认为是等价的，第一个使用事件机制实现，第二个则使用回调来通知调用者，而将事件作为参数传递。但是真正区分它们的是可执行性，语义和要实现或使用的代码量。虽然我们不能给出一套确定性的规则来选择一种风格，但我们当然可以提供一些提示来帮助你做出决定。

相比于第一个例子，即观察者模式而言，回调函数在支持不同类型的事件时有一些限制。但是事实上，我们仍然可以通过将事件类型作为回调的参数传递，或者通过接受多个回调来区分多个事件。然而，这样做的话不能被认为是一个优雅的`API`。在这种情况下，`EventEmitter`可以提供更好的接口和更精简的代码。

`EventEmitter`更优秀的另一种应用场景是多次触发同一事件或不触发事件的情况。事实上，无论操作是否成功，一个回调预计都只会被调用一次。但有一种特殊情况是，我们可能不知道事件在哪个时间点触发，在这种情况下，`EventEmitter`是首选。

最后，使用回调的`API`仅通知特定的回调，但是使用`EventEmitter`函数可以让多个监听器都接收到通知。

### 回调机制和事件机制结合使用
还有一些情况可以将事件机制和回调结合使用。特别是当我们导出异步函数时，这种模式非常有用。[node-glob模块](https://npmjs.org/package/glob)是该模块的一个示例。

```javascript
glob(pattern, [options], callback)
```

该函数将一个文件名匹配模式作为第一个参数，后面两个参数分别为一组选项和一个回调函数，对于匹配到指定文件名匹配模式的文件列表，相关回调函数会被调用。同时，该函数返回`EventEmitter`，它展现了当前进程的状态。例如，当成功匹配文件名时可以实时发布`match`事件，当文件列表全部匹配完毕时可以实时发布`end`事件，或者该进程被手动中止时发布`abort`事件。看以下代码：

```javascript
const glob = require('glob');
glob('data/*.txt', (error, files) => console.log(`All files found: ${JSON.stringify(files)}`))
  .on('match', match => console.log(`Match found: ${match}`));
```

## 总结
在本章中，我们首先了解了同步和异步的区别。然后，我们探讨了如何使用回调机制和回调机制来处理一些基本的异步方案。我们还了解到两种模式之间的主要区别，何时比另一种模式更适合解决具体问题。我们只是迈向更先进的异步模式的第一步。

在下一章中，我们将介绍更复杂的场景，了解如何利用回调机制和事件机制来处理高级异步控制问题。