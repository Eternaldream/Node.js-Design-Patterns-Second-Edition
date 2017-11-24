# Coding with Streams
`Streams`是`Node.js`最重要的组件和模式之一。 社区中有一句格言“Stream all the things（Steam就是所有的）”，仅此一点就足以描述流在`Node.js`中的地位。 `Dominic Tarr`作为`Node.js`社区的最大贡献者，它将流定义为`Node.js`最好，也是最难以理解的概念。

使`Node.js`的`Streams`如此吸引人还有其它原因; 此外，`Streams`不仅与性能或效率等技术特性有关，更重要的是它们的优雅性以及它们与`Node.js`的设计理念完美契合的方式。

在本章中，将会学到以下内容：
* `Streams`对于`Node.js`的重要性。
* 如何创建并使用`Streams`。
* `Streams`作为编程范式，不只是对于`I/O`而言，在多种应用场景下它的应用和强大的功能。
* 管道模式和在不同的配置中连接`Streams`。

## 发现Streams的重要性
在基于事件的平台（如`Node.js`）中，处理`I / O`的最有效的方法是实时处理，一旦有输入的信息，立马进行处理，一旦有需要输出的结果，也立马输出反馈。

在本节中，我们将首先介绍`Node.js`的`Streams`和它的优点。 请记住，这只是一个概述，因为本章后面将会详细介绍如何使用和组合`Streams`。

### Streams和Buffer的比较
我们在本书中几乎所有看到过的异步API都是使用的`Buffer`模式。 对于输入操作，`Buffer`模式会将来自资源的所有数据收集到`Buffer`区中; 一旦读取完整个资源，就会把结果传递给回调函数。 下图显示了这个范例的一个真实的例子：

![](http://oczira72b.bkt.clouddn.com/17-11-22/20243023.jpg)

从上图我们可以看到，在**t1**时刻，一些数据从资源接收并保存到缓冲区。 在**t2**时刻，最后一段数据被接收到另一个数据块，完成读取操作，这时，把整个缓冲区的内容发送给消费者。

另一方面，`Streams`允许你在数据到达时立即处理数据。 如下图所示：

![](http://oczira72b.bkt.clouddn.com/17-11-22/5372675.jpg)

这一张图显示了`Streams`如何从资源接收每个新的数据块，并立即提供给消费者，消费者现在不必等待缓冲区中收集所有数据再处理每个数据块。

但是这两种方法有什么区别呢？ 我们可以将它们概括为两点：

* 空间效率
* 时间效率

此外，`Node.js`的`Streams`具有另一个重要的优点：**可组合性（composability）**。 现在让我们看看这些属性对我们设计和编写应用程序的方式会产生什么影响。

### 空间效率
首先，`Streams`允许我们做一些看起来不可能的事情，通过缓冲数据并一次性处理。 例如，考虑一下我们必须读取一个非常大的文件，比如说数百`MB`甚至千`MB`。 显然，等待完全读取文件时返回大`Buffer`的`API`不是一个好主意。 想象一下，如果并发读取一些大文件， 我们的应用程序很容易耗尽内存。 除此之外，`V8`中的`Buffer`不能大于`0x3FFFFFFF`字节（小于`1GB`）。 所以，在耗尽物理内存之前，我们可能会碰壁。

### 使用Buffered的API进行压缩文件
举一个具体的例子，让我们考虑一个简单的命令行接口（`CLI`）的应用程序，它使用`Gzip`格式压缩文件。 使用`Buffered`的`API`，这样的应用程序在`Node.js`中大概这么编写（为简洁起见，省略了异常处理）：

```javascript
const fs = require('fs');
const zlib = require('zlib');
const file = process.argv[2];
fs.readFile(file, (err, buffer) => {
  zlib.gzip(buffer, (err, buffer) => {
    fs.writeFile(file + '.gz', buffer, err => {
      console.log('File successfully compressed');
    });
  });
});
```

现在，我们可以尝试将前面的代码放在一个叫做`gzip.js`的文件中，然后执行下面的命令：

```bash
node gzip <path to file>
```

如果我们选择一个足够大的文件，比如说大于`1GB`的文件，我们会收到一个错误信息，说明我们要读取的文件大于最大允许的缓冲区大小，如下所示：

```
RangeError: File size is greater than possible Buffer:0x3FFFFFFF
```

![](http://oczira72b.bkt.clouddn.com/17-11-22/34259823.jpg)

> 上面的例子中，没找到一个大文件，但确实对于大文件的读取速率慢了许多。

正如我们所预料到的那样，使用`Buffer`来进行大文件的读取显然是错误的。

### 使用Streams进行压缩文件
我们必须修复我们的`Gzip`应用程序，并使其处理大文件的最简单方法是使用`Streams`的`API`。 让我们看看如何实现这一点。 让我们用下面的代码替换刚创建的模块的内容：

```javascript
const fs = require('fs');
const zlib = require('zlib');
const file = process.argv[2];
fs.createReadStream(file)
  .pipe(zlib.createGzip())
  .pipe(fs.createWriteStream(file + '.gz'))
  .on('finish', () => console.log('File successfully compressed'));
```

“是吗？”你可能会问。是的；正如我们所说的，由于`Streams`的接口和可组合性，因此我们还能写出这样的更加简洁，优雅和精炼的代码。 我们稍后会详细地看到这一点，但是现在需要认识到的重要一点是，程序可以顺畅地运行在任何大小的文件上，理想情况是内存利用率不变。 尝试一下（但考虑压缩一个大文件可能需要一段时间）。

### 时间效率
现在让我们考虑一个压缩文件并将其上传到远程`HTTP`服务器的应用程序的例子，该远程`HTTP`服务器进而将其解压缩并保存到文件系统中。如果我们的客户端是使用`Buffered`的`API`实现的，那么只有当整个文件被读取和压缩时，上传才会开始。 另一方面，只有在接收到所有数据的情况下，解压缩才会在服务器上启动。 实现相同结果的更好的解决方案涉及使用`Streams`。 在客户端机器上，`Streams`只要从文件系统中读取就可以压缩和发送数据块，而在服务器上，只要从远程对端接收到数据块，就可以解压每个数据块。 我们通过构建前面提到的应用程序来展示这一点，从服务器端开始。

我们创建一个叫做`gzipReceive.js`的模块，代码如下：

```javascript
const http = require('http');
const fs = require('fs');
const zlib = require('zlib');

const server = http.createServer((req, res) => {
  const filename = req.headers.filename;
  console.log('File request received: ' + filename);
  req
    .pipe(zlib.createGunzip())
    .pipe(fs.createWriteStream(filename))
    .on('finish', () => {
      res.writeHead(201, {
        'Content-Type': 'text/plain'
      });
      res.end('That\'s it\n');
      console.log(`File saved: ${filename}`);
    });
});

server.listen(3000, () => console.log('Listening'));
```

服务器从网络接收数据块，将其解压缩，并在接收到数据块后立即保存，这要归功于`Node.js`的`Streams`。

我们的应用程序的客户端将进入一个名为`gzipSend.js`的模块，如下所示：

在前面的代码中，我们再次使用`Streams`从文件中读取数据，然后在从文件系统中读取的同时压缩并发送每个数据块。

现在，运行这个应用程序，我们首先使用以下命令启动服务器：

```bash
node gzipReceive
```

然后，我们可以通过指定要发送的文件和服务器的地址（例如`localhost`）来启动客户端：

```bash
node gzipSend <path to file> localhost
```

![](http://oczira72b.bkt.clouddn.com/17-11-22/86343554.jpg)

如果我们选择一个足够大的文件，我们将更容易地看到数据如何从客户端流向服务器，但为什么这种模式下，我们使用`Streams`，比使用`Buffered`的`API`更有效率？ 下图应该给我们一个提示：

![](http://oczira72b.bkt.clouddn.com/17-11-22/17710786.jpg)

一个文件被处理的过程，它经过以下阶段：

1. 客户端从文件系统中读取
2. 客户端压缩数据
3. 客户端将数据发送到服务器
4. 服务端接收数据
5. 服务端解压数据
6. 服务端将数据写入磁盘 

为了完成处理，我们必须按照流水线顺序那样经过每个阶段，直到最后。在上图中，我们可以看到，使用`Buffered`的`API`，这个过程完全是顺序的。为了压缩数据，我们首先必须等待整个文件被读取完毕，然后，发送数据，我们必须等待整个文件被读取和压缩，依此类推。当我们使用`Streams`时，只要我们收到第一个数据块，流水线就会被启动，而不需要等待整个文件的读取。但更令人惊讶的是，当下一块数据可用时，不需要等待上一组任务完成；相反，另一条装配线是并行启动的。因为我们执行的每个任务都是异步的，这样显得很完美，所以可以通过`Node.js`来并行执行`Streams`的相关操作；唯一的限制就是每个阶段都必须保证数据块的到达顺序。

从前面的图可以看出，使用`Streams`的结果是整个过程花费的时间更少，因为我们不用等待所有数据被全部读取完毕和处理。

### 组合性
到目前为止，我们已经看到的代码已经告诉我们如何使用`pipe()`方法来组装`Streams`的数据块，`Streams`允许我们连接不同的处理单元，每个处理单元负责单一的职责（这是符合`Node.js`风格的）。这是可能的，因为`Streams`具有统一的接口，并且就`API`而言，不同`Streams`也可以很好的进行交互。唯一的先决条件是管道的下一个`Streams`必须支持上一个`Streams`生成的数据类型，可以是二进制，文本甚至是对象，我们将在后面的章节中看到。

为了证明`Streams`组合性的优势，我们可以尝试在我们先前构建的`gzipReceive / gzipSend`应用程序中添加加密功能。
为此，我们只需要通过向流水线添加另一个`Streams`来更新客户端。 确切地说，由`crypto.createChipher()`返回的流。 由此产生的代码应如下所示：

```javascript
const fs = require('fs');
const zlib = require('zlib');
const crypto = require('crypto');
const http = require('http');
const path = require('path');

const file = process.argv[2];
const server = process.argv[3];

const options = {
  hostname: server,
  port: 3000,
  path: '/',
  method: 'PUT',
  headers: {
    filename: path.basename(file),
    'Content-Type': 'application/octet-stream',
    'Content-Encoding': 'gzip'
  }
};

const req = http.request(options, res => {
  console.log('Server response: ' + res.statusCode);
});

fs.createReadStream(file)
  .pipe(zlib.createGzip())
  .pipe(crypto.createCipher('aes192', 'a_shared_secret'))
  .pipe(req)
  .on('finish', () => {
    console.log('File successfully sent');
  });
```

使用相同的方式，我们更新服务端的代码，使得它可以在数据块进行解压之前先解密：

```javascript
const http = require('http');
const fs = require('fs');
const zlib = require('zlib');
const crypto = require('crypto');

const server = http.createServer((req, res) => {
  const filename = req.headers.filename;
  console.log('File request received: ' + filename);
  req
    .pipe(crypto.createDecipher('aes192', 'a_shared_secret'))
    .pipe(zlib.createGunzip())
    .pipe(fs.createWriteStream(filename))
    .on('finish', () => {
      res.writeHead(201, {
        'Content-Type': 'text/plain'
      });
      res.end('That\'s it\n');
      console.log(`File saved: ${filename}`);
    });
});

server.listen(3000, () => console.log('Listening'));
```

> [crypto](https://nodejs.org/dist/latest-v9.x/docs/api/crypto.html#crypto_crypto)是Node.js的核心模块之一，提供了一系列加密算法。

只需几行代码，我们就在应用程序中添加了一个加密层。 我们只需要简单地通过把已经存在的`Streams`模块和加密层组合到一起，就可以。类似的，我们可以添加和合并其他`Streams`，如同在玩乐高积木一样。

显然，这种方法的主要优点是可重用性，但正如我们从目前为止所介绍的代码中可以看到的那样，`Streams`也可以实现更清晰，更模块化，更加简洁的代码。 出于这些原因，流通常不仅仅用于处理纯粹的`I / O`，而且它还是简化和模块化代码的手段。

## 开始使用Streams
在前面的章节中，我们了解了为什么`Streams`如此强大，而且它在`Node.js`中无处不在，甚至在`Node.js`的核心模块中也有其身影。 例如，我们已经看到，`fs`模块具有用于从文件读取的`createReadStream()`和用于写入文件的`createWriteStream()`，`HTTP`请求和响应对象本质上是`Streams`，并且`zlib`模块允许我们使用`Streams`式`API`压缩和解压缩数据块。

现在我们知道为什么`Streams`是如此重要，让我们退后一步，开始更详细地探索它。

### Streams的结构
`Node.js`中的每个`Streams`都是`Streams`核心模块中可用的四个基本抽象类之一的实现：

* `stream.Readable`
* `stream.Writable`
* `stream.Duplex`
* `stream.Transform`

每个`stream`类也是`EventEmitter`的一个实例。实际上，`Streams`可以产生几种类型的事件，比如`end`事件会在一个可读的`Streams`完成读取，或者错误读取，或其过程中产生异常时触发。

> 请注意，为简洁起见，在本章介绍的例子中，我们经常会忽略适当的错误处理。但是，在生产环境下中，总是建议为所有Stream注册错误事件侦听器。

`Streams`之所以如此灵活的原因之一是它不仅能够处理二进制数据，而且几乎可以处理任何`JavaScript`值。实际上，`Streams`可以支持两种操作模式：

* 二进制模式：以数据块形式（例如`buffers`或`strings`）流式传输数据
* 对象模式：将流数据视为一系列离散对象（这使得我们几乎可以使用任何`JavaScript`值）

这两种操作模式使我们不仅可以使用`I / O`流，而且还可以作为一种工具，以函数式的风格优雅地组合处理单元，我们将在本章后面看到。

> 在本章中，我们将主要使用在Node.js 0.11中引入的Node.js流接口，也称为版本3。 有关与旧接口差异的更多详细信息，请参阅StrongLoop在https://strongloop.com/strongblog/whats-new-io-js-beta-streams3/中的优秀博客文章。

### 可读的Streams
一个可读的`Streams`表示一个数据源，在`Node.js`中，它使用`stream`模块中的`Readableabstract`类实现。

#### 从Streams中读取信息
从可读`Streams`接收数据有两种方式：`non-flowing`模式和`flowing`模式。 我们来更详细地分析这些模式。

##### non-flowing模式（不流动模式）
从可读的`Streams`中读取数据的默认模式是为其附加一个可读事件侦听器，用于指示要读取的新数据的可用性。然后，在一个循环中，我们读取所有的数据，直到内部`buffer`被清空。这可以使用`read()`方法完成，该方法同步从内部缓冲区中读取数据，并返回表示数据块的`Buffer`或`String`对象。`read()`方法以如下使用模式：

```javascript
readable.read([size]);
```

使用这种方法，数据随时可以直接从`Streams`中按需提取。

为了说明这是如何工作的，我们创建一个名为`readStdin.js`的新模块，它实现了一个简单的程序，它从标准输入（一个可读流）中读取数据，并将所有数据回送到标准输出：

```javascript
process.stdin
  .on('readable', () => {
    let chunk;
    console.log('New data available');
    while ((chunk = process.stdin.read()) !== null) {
      console.log(
        `Chunk read: (${chunk.length}) "${chunk.toString()}"`
      );
    }
  })
  .on('end', () => process.stdout.write('End of stream'));
```

`read()`方法是一个同步操作，它从可读`Streams`的内部`Buffers`区中提取数据块。如果`Streams`在二进制模式下工作，返回的数据块默认为一个`Buffer`对象。

> 在以二进制模式工作的可读的Stream中，我们可以通过在Stream上调用setEncoding(encoding)来读取字符串而不是Buffer对象，并提供有效的编码格式（例如utf8）。

数据是从可读的侦听器中读取的，只要有新的数据，就会调用这个侦听器。当内部缓冲区中没有更多数据可用时，`read()`方法返回`null`；在这种情况下，我们不得不等待另一个可读的事件被触发，告诉我们可以再次读取或者等待表示`Streams`读取过程结束的`end`事件触发。当一个流以二进制模式工作时，我们也可以通过向`read()`方法传递一个`size`参数来指定我们想要读取的数据大小。这在实现网络协议或解析特定数据格式时特别有用。

现在，我们准备运行`readStdin`模块并进行实验。让我们在控制台中键入一些字符，然后按`Enter`键查看回显到标准输出中的数据。要终止流并因此生成一个正常的结束事件，我们需要插入一个`EOF`（文件结束）字符（在`Windows`上使用`Ctrl + Z`或在`Linux`上使用`Ctrl + D`）。

我们也可以尝试将我们的程序与其他程序连接起来;这可以使用管道运算符（`|`），它将程序的标准输出重定向到另一个程序的标准输入。例如，我们可以运行如下命令：

```bash
cat <path to a file> | node readStdin
```

这是流式范例是一个通用接口的一个很好的例子，它使得我们的程序能够进行通信，而不管它们是用什么语言写的。

##### flowing模式（流动模式）
