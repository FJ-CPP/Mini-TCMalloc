# Mini-TCMalloc

## 一、前言

> TCMalloc(或tcmalloc)，全称Thread-Caching-Malloc，即线程缓存的malloc，是一个由Google开源的内存分配器项目，在Golang中使用类似算法进行内存分配。

与 glibc 标准库中实现的 malloc 相比，TCMalloc 在内存的分配上效率和速度要高得多，且在**多线程下表现尤为突出**，因此常用来优化C++编写的高并发项目。

## 二、项目简介

Mini-TCMalloc是基于tcmalloc的核心部分实现的高并发内存池(**适用于WIN32平台**)，通过完成**相关数据结构**和**互斥锁**的设计，能够更加深入地理解“**并发高效**”背后的秘密，同时有助于提高数据结构的运用能力和并发的设计能力。

本项目主要包括以下核心内容：

1. 池化技术
2. 数据结构(链表、哈希桶、基数树)
3. 单例模式
4. 多线程
5. 互斥锁

## 三、架构

### 1、内存申请之Thread Cache

> Thread Cache是**每个线程都拥有**且**相互独立**的线程级内存池，通过**哈希表**来管理不同大小的**内存块链表**(哈希桶)。

![image-20220803152451400](https://mypicture-1307604235.cos.ap-nanjing.myqcloud.com/mytyporaimage-20220803152451400.png)

当线程申请`size`大小的内存时，首先通过**内存对齐算法**将`size`对齐至`alignSize`，再到哈希表的`alignSize`处取下一个对应大小的内存块即可。

#### I.内存对齐和碎片率

`[1, 256*1024]`字节的内存块通过内存对齐后进行分配，大于`256*1024`字节的内存块直接向Central Cache申请。

|          size           |   对齐数    | 对齐后的alignSize在哈希表中的下标 |
| :---------------------: | :---------: | :-------------------------------: |
|        [1, 128]         |    8Byte    |              [0, 16)              |
|      [128+1, 1024]      |   16Byte    |             [16, 72)              |
|    [1024+1, 8\*1024]    |   128Byte   |             [72, 128)             |
|  [8\*1024+1, 64\*1024]  |  1024Byte   |            [128, 184)             |
| [64\*1024+1, 256\*1024] | 8\*1024Byte |             [184,208)             |

以申请`64*1024+1`字节为例，根据对齐数`8*1024`，该大小最终被对齐至`72*1024`字节，相当于浪费了`7*1024+1023`字节，计算**内存碎片率**为：`(7*1024+1023) / (72*1024) * 100% ≈ 11%`

通过计算其它对齐数的情况可以得出结论：*在最坏情况下tcmalloc的对齐规则可以将内存碎片率控制在==11%==左右！*

> 注：
>
> malloc的内存对齐数为8字节，拥有更低的内存碎片率，但是在tcmalloc这里行不通。因为将[1, 256\*1024]按照8字节分割的话，将会分割出256\*128种内存块，如果讲这些内存块存至哈希表，对内存的负担过重！
>
> 因此，权衡考虑，tcmalloc的这种对齐规则已经非常优异。

#### II.在哈希表中的下标计算

[1, 128]对齐数为8，因此8个为一组，共有`128/8=16`个，因此对应在哈希表中的下标为[0, 16)

[128+1, 1024]对齐数为16，因此16个为一组，共有`(1024-128)/16=56`个，因此对应在哈希表中的下标就为[16, 16+56)

其余依次类推，最终结果为表格第三列。

#### III.空闲内存块管理

不同大小的内存被存放在哈希表对应位置的链表中，其组织结构为：

![image-20220802125218472](https://mypicture-1307604235.cos.ap-nanjing.myqcloud.com/mytyporaimage-20220802125218472.png)

通过前4/8个字节存储下一个内存块的起始地址，将所有内存块连接起来。

具体前4个字节还是8个字节需要根据平台的指针大小而定。

#### IV.TLS

即`Thread Local Storage`。

Mini-TCMalloc封装了`TLSThreadCache`类，并将其设置成thread_local类型。

`TLSThreadCache`为每个线程创建一个“**线程本地存储**”的`Thread Cache`对象。其他线程无法访问本线程的`Thread Cache`，从而保证了==独享==，<font color=red>避免了加解锁的消耗</font>！

C++11提供了`threadlocal关键字`来定义线程本地存储对象。

### 2、内存申请之Central Cache

> 中央缓存Central Cache是一个**全局唯一**的内存管理模块，通过与Thread Cache相同结构的**哈希表**来管理不同大小的**内存块链表**(哈希桶)。但是不同的是，哈希桶内存储的不再是一个个小的内存块，而是可以分割成小内存块的`Span`。
>

#### I.Span结构

每种`size`都会被对齐至`alignSize`，而不同`alignSize`的内存块需要从不同大小的`Span`中切割下来。

每个`Span`是由不同数量的连续页构成的，它们通过**双链表**管理起来。

![image-20220803122013982](https://mypicture-1307604235.cos.ap-nanjing.myqcloud.com/mytyporaimage-20220803122013982.png)

当Thread Cache向Central Cache申请小内存块时，步骤如下：

1. 根据size计算出哈希桶下标(该规则与Thread Cache相同)
2. 到Central Cache的对应哈希桶中找到一个可用的Span
3. 将该Span分割出小的内存块并返回给Thread Cache

#### II.批量申请与慢启动

Thread Cache为每一种大小的内存块设定了一个**慢启动阈值maxLength**。

当用户申请size大小的空间，且Thread Cache的空闲链表为空时，Thread Cache并非一次只向Central Cache申请一个内存块，而是计算一个**理论申请值**，并将它与慢启动阈值比较。

- 若理论值>阈值，则按照阈值批量申请，同时阈值+1(慢启动调节)
- 若理论值<=阈值，则按照理论值批量申请

在慢启动的干涉下，Thread Cache不必频繁地向Central Cache申请内存块，也在一定程度上缓解了申请过多内存块而用不完的情况。

### 3、内存申请之Page Heap

![image-20220803151924741](https://mypicture-1307604235.cos.ap-nanjing.myqcloud.com/mytyporaimage-20220803151924741.png)

> Page Heap同样通过哈希桶管理着Span结构，Span内包含的页数范围为[1, 128]，对应哈希表的下标[1, 128]

当Central Cache向Page Heap申请npage页的Span时，Page Heap顺着[npage, 128]遍历哈希桶，找到第一个符合条件的Span，假设其大小为kpage，则将其分割成npage和kpage-npage的两个Span，将npage返回给Central Cache并将kpage-npage插入至对应的哈希桶处。

如果哈希桶内不存在符合要求的Span，则Page Heap向系统申请一个128页的Span，并再次进行上述过程。

与Central Cache不同之处在于：

1. Page Heap管理的**Span是没有被切分的**，它负责向Central Cache提供完好的、没有被使用的Span。
2. Page Heap**无法使用桶锁**，因为它可能需要遍历哈希桶才能找到符合要求的Span，而每遍历一个桶都要加一次锁，效率太低！

#### 大块内存申请

如果申请内存大于256KB，则向Page Heap申请：

1. 不超过128页，则直接从哈希桶中取
2. 超过128页，直接向系统申请

## 四、内存回收

### 1、Thread Cache

线程释放的内存块首先返还给Thread Cache对应的哈希桶中，同时检测哈希桶的长度：

1. 如果哈希桶长度`length`大于等于当前的慢启动阈值`maxLength`，则将`maxLength`个内存块返还给上层的`Central Cache`。
2. 如果哈希桶长度`length`小于当前的慢启动阈值`maxLength`，则本次回收结束。

此外，在线程结束时，线程本地存储的`TLSThreadCache`自动调用Thread Cache的析构函数，将其内部的所有内存块归还给`Central Cache`。

### 2、Central Cache

Central Cache负责回收Thread Cache传过来的内存块，并将其归入到它原本属于的Span中(**本质上就是Span的已使用内存块数量`useCount-1`**)。

1. 如果Span.useCount=0，说明它分出去的所有内存块都已归还，此时再将这个Span归还给Page Heap。
2. 如果Span.useCount!=0，则无需处理。

### 3、Page Heap

Page Heap负责两件事：

1. 回收大于256KB的内存，如果是不超过128页的，则将其归入对应的哈希桶，如果超过128页，则直接还给系统堆。
2. 回收Central Cache传过来的Span，并尝试合并出更大的Span。

## 五、关键实现说明

### 1、页号PAGE_ID

> 内存页的存在本质上将内存按一定大小分成一个个虚拟页。

宏`PAGE_SHIFT`用来定义页的大小，即一页`1<<PAGE_SHIFT字节`：当PAGE_SHIFT取13页，一页就是`1<<13=8KB`。

按照一页8KB，则地址范围为***0\~8KB-1***为0号页，***8KB\~16KB-1***为1号页，***16KB\~24KB-1***为3号页，以此类推...

32位系统下，地址范围为***0~2^32^-1***，因此最大的页号就是`2^32 << PAGE_SHIFT - 1，即2^19-1`。

### 2、PAGE_ID : Span的映射

> 注：由于本项目针对WIN32，因此这里仅讨论32位系统的情况。

Span是由连续的页面构成的，因此一个Span可能对应着一个或多个页面ID。32位下TCMalloc使用`两层基数树radix tree`来维护PAGE_ID到Span的映射关系。

`radix tree`采取与多层页表类似的方式进行映射：

- 树的第一层映射PAGE_ID的**前5个位**，因此第一层共有**2^5^**个槽位，每个位置存放第二层的结点指针。
- 树的第二层映射PAGE_ID的**后14个位**(由于页号最大为2^19^-1，因此页号最多有19位)，每个节点的对应位置存放的就是对应的Span。

![image-20220805110101427](https://typora-1307604235.cos.ap-nanjing.myqcloud.com/typora_img/202208051854671.png)

基数树可以达到接近于**直接定址法**的效率，还可以确保只在需要的时候开辟空间进行映射，因此更加节省空间。

### 3、Span的合并

对于Page Heap中已分配出去的Span，需要先保存Span内部所有页的ID到Span的映射关系。

![image-20220805110605073](https://typora-1307604235.cos.ap-nanjing.myqcloud.com/typora_img/202208051854391.png)

对于未分配出去的Span，只需要保存起始页和最后一页的ID到Span的映射关系。

当Central Cache将Span还回来时，需要完成：

1. 检测Span的前一页所在的Span，如果存在且没有分配出去，则将这两个Span合并，然后继续执行步骤1。
2. 检测Span的最后一个的后一页所在的Span，如果存在且没有被分配出，则将这两个Span合并，然后继续执行步骤2。
3. 当合并的过程中出现两页大小之和超过128页时(即Page Heap能管理的最大页数)，则停止合并。

### 4、定长内存池介入

在Mini-TCMalloc中，包括Thread Cache、Span在内的数据结构都需要额外动态申请空间。为了完全脱离malloc和free，这里实现了一个定长的内存池`ObjectPool`来申请和释放这些定长的数据结构。

`ObjectPool`会利用系统调用申请一大块的空间，在需要时从大块空间中切出一指定大小的一部分给用户。

![image-20220805142531296](https://typora-1307604235.cos.ap-nanjing.myqcloud.com/typora_img/202208051854164.png)

同时，`ObjectPool`像Thread Cache那样维护空闲链表，将外部返还回来的内存块管理起来，方便以后直接从链表中取内存块返还给用户。

## 六、基准测试对比

> 测试环境：Visual Studio 2019-x86-Release
>
> 测试代码：见Benchmark.cpp文件

### 1、单线程随机内存大小申请测试

![image-20220805140950172](https://typora-1307604235.cos.ap-nanjing.myqcloud.com/typora_img/202208051854655.png)

![image-20220805140928903](https://typora-1307604235.cos.ap-nanjing.myqcloud.com/typora_img/202208051854220.png)

在运行之初，tcmalloc的效率是malloc的40倍左右，后期稳定在4~6倍。

### 2、多线程随机内存大小申请测试



![image-20220805140609337](https://typora-1307604235.cos.ap-nanjing.myqcloud.com/typora_img/202208051854437.png)

![image-20220805140751338](https://typora-1307604235.cos.ap-nanjing.myqcloud.com/typora_img/202208051854357.png)

可以看到，Mini-TCMalloc的表现非常优异，在高并发情况下稳定在malloc速度的10~20倍。

### 3、多线程指定内存大小申请测试

![image-20220805141834709](https://typora-1307604235.cos.ap-nanjing.myqcloud.com/typora_img/202208051854150.png)

![image-20220805141840513](https://typora-1307604235.cos.ap-nanjing.myqcloud.com/typora_img/202208051854009.png)

前期略显优势，但是因为桶竞争问题，导致tcmalloc整体性能略逊一筹。但是考虑到测试场景比较特殊，现实工程中一般会采取定长的数据结构池来负责解决这种情况。

## 七、结语

设计本项目的目的一方面是理解一个优秀的内存分配器，另一方面也是对学习数据结构算法至今的一个检验。

此外，为了解决项目中的各种内存和并发问题所做出的努力，也在一定程度上增长了自身的调试经验。

总之，收获颇丰！
