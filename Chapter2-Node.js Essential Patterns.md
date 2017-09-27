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
