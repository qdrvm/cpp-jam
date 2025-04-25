These features were rejected, don't suggest them again.

```c++
#define FORWARD(x) std::forward<decltype(x)>(x)
auto foo(auto &&x) {
  bar(FORWARD(x));
}
```

```c++
#define MAKE_SHARED_(x_, ...) std::make_shared<decltype(x_)::element_type>(...)
struct Foo {
  Foo() : MAKE_SHARED_(bar_, ...) {}
  std::shared_ptr<Bar> bar_;
};
```

```c++
#define MAKE_SHARED_T(x) std::forward<decltype(x)>(x)
using Foo = std::shared_ptr<...>;
MAKE_SHARED_T(Foo, ...);
```

```c++
#define MOVE(x) x{std::move(x)}
auto lambda = [MOVE(x)]() {};
```

```c++
#define MOVE_(x) x##_{std::move(x)}
struct Foo {
  Foo(Bar bar) : MOVE_(bar) {}
  Bar bar_;
};
```

```c++
#define WEAK_SELF weak_self{weak_from_this()}
auto lambda = [WEAK_SELF]() {};
```

```c++
#define WEAK_LOCK(x) auto x = weak_##x.lock(); if (not x) return
auto lambda = [weak_self]() {
  WEAK_LOCK(self);
  ...
};
```

```c++
class MakeSharedPrivate {
 private:
  MakeSharedPrivate() = default;
 public:
  static auto make() {
    return MakeSharedPrivate{};
  }
};
class Foo : public std::enable_shared_from_this<> {
 public:
  Foo(MakeSharedPrivate, ...);
  static std::shared_ptr<Foo> factory(...) {
    // `Foo` can call `MakeSharedPrivate` constructor
    return std::make_shared<Foo>(MakeSharedPrivate::make(), ...);
  }
};
// error, MakeSharedPrivate has private constructor
injector.create<std::shared_ptr<Foo>>();
```
