# libsweeper

An simple implementation of a Mark and Sweep garbage collector, used in
[TerrorVM][terrorvm] but completely agnostic.

## Using

### Installing it as a submodule

    $ cd <your_project>
    $ mkdir -p deps
    $ git submodule add git://github.com/txus/libsweeper deps/libsweeper

In your Makefile, add this to your `CFLAGS`:

    CFLAGS=<whatever> -Ideps/libsweeper/include

And this to your LDFLAGS:

    LDFLAGS=<whatever> deps/libsweeper/build/libsweeper.a

Create a `gc` target in your Makefile:

```make
gc:
  $(MAKE) -C deps/libsweeper
```

And finally add it as a dependency of your main target:

```make
all: gc <your> <other> <targets>
```

Done! :)

### Interface

Interfacing your objects with `libsweeper` is easy. You just need to add a
header field in your **object** struct (or values or however you call them).

As easily seen [in the tests][tests], here's an example Object struct:

```c
#include <sweeper/header.h>

typedef struct Object_s {
  SWPHeader gc;
  ... // your own things
} Object;
```

The `SWPHeader` **must** be the first field in the struct. This is important so
that the GC can handle your objects as if they were `SWPHeader` types, since it
knows nothing about your object layout.

#### Functions to define

You need to provide three custom functions for the GC to work:

* A function to **destroy** an object of yours, with signature `void (SWPHeader*)`
* A function to **determine the rootset**, with signature `void (void*,
  SWPArray*)`
* A function to **determine the references of an object**, with signature `void (SWPHeader*, SWPArray*)`

Note the difference between `SWPHeader` and `SWPArray`. The former is an
object header, whereas the latter is a collection of object headers.

Here's an example of how those functions might look:

```c
#include <sweeper/header.h>

// Function to destroy an object
void my_destroy(SWPHeader *obj) {
  free(obj);
}

// Function to determine the rootset
void my_add_roots(void *state, SWPArray *roots) {
  State *my_state = (State*)state;
  // find out the rootset, scanning the stack, global variables, etc
  // and for each result do:
  SWPArray(roots, object);
}

// Function to determine the references of an object
void my_add_children(SWPHeader *obj, SWPArray *children) {
  Object *object = (Object*)obj;
  // find out the objects referenced by `object` and for each do:
  SWPArray_push(children, referenced_object);
}
```

#### Creating the Heap object

Here's the signature of the constructor function for an SWPHeap:

```c
// in <sweeper/sweeper.h>
SWPHeap* SWPHeap_new(
  unsigned int size,
  unsigned int max_size,
  double growth_factor,
  void *state,
  size_t object_size,
  SWPReleaseFn release,
  SWPAddRootsFn add_roots,
  SWPAddChildrenFn add_children
  );
```

* **size** is initial size of the heap, in number of objects.
* **max_size** is the max size of the heap, in number of objects.
* **growth_factor** is the factor at which the heap will grow after each
  collection, to make collections less frequent. Set this to some sane value such as `1.8`.
* **state** is a pointer to your State object or whatever you use to determine
  the rootset.
* **object_size** is the size of your object struct, for example
  `sizeof(Object)`.
* **release** is the function you defined earlier to destroy your objects.
* **add_roots** is the function you defined earlier to determine the rootset.
* **add_children** is the function you defined earlier to determine the
  references of an object.

You're all set! You got a new shiny garbage-collected heap :)

Keep a reference to the heap wherever you deem best (normally your State
object). Every time you need to **allocate an object**, do this:

```c
Object *my_obj = (Object*)swp_allocate(heap);
```

And finally, at the end of your program, remember to destroy the heap:

```c
SWPHeap_destroy(heap);
```

Destroying the heap **won't release** your live objects at that point. Make sure
to empty the rootset and run a last `swp_collect(heap)` before you destroy the
heap.

## Development

To build libsweeper and run its test suite:

    $ git clone git://github.com/txus/libsweeper
    $ cd libsweeper
    $ make

If you want to run with Valgrind to ensure there are no memory leaks:

    $ make valgrind

Tested in Mac OSX 10.8.

## Who's this

This was made by [Josep M. Bach (Txus)](http://txustice.me) under the MIT
license. I'm [@txustice][twitter] on twitter (where you should probably follow
me!).

## Contributing

1. Fork it
2. Create your feature branch (`git checkout -b my-new-feature`)
3. Commit your changes (`git commit -am 'Added some feature'`)
4. Push to the branch (`git push origin my-new-feature`)
5. Create new Pull Request

[twitter]: https://twitter.com/txustice
[terrorvm]: https://github.com/txus/terrorvm
[tests]: https://github.com/txus/libsweeper/blob/master/tests/sweeper_tests.c