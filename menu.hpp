#pragma once

namespace menu {
  typedef unsigned char uchar;
  
  struct Printer {
    bool blink;
    void print(const char *message);
    void print(char character);
    void print(int value);
    void print(float value, int digits);
    void println();
  };

  class AbstractItem {
  public:
    virtual void draw_value(Printer &out) const;
    virtual void draw_screen(Printer &out, uchar lines_count) const;
    virtual void exec(char command);
  protected:
    virtual void draw_simple_value(Printer &out) const;
  };

  extern AbstractItem *focused;

  class FocusableItem: public AbstractItem {
    typedef AbstractItem super;
  public:
    void draw_screen(Printer &out, uchar lines_count) const;
    void exec(char command);
  protected:
    AbstractItem *previous;
  };

  class List: public FocusableItem {
    typedef FocusableItem super;
  public:
    static const char *BACK_TEXT;
    struct Item {
      const char* label;
      AbstractItem &item;
    };

    template<uchar count> static List from(Item(&items)[count]) {
      return List(items, count);
    }

    explicit List(Item *items, uchar count);

    void draw_screen(Printer &out, uchar lines_count) const;
    void exec(char command);
  public:
    Item *items;
    uchar count, max_index;
    uchar cursor, offset;
  };

  template<typename T>
  struct remove_reference { typedef T type; };
  template<typename T>
  struct remove_reference<T&> { typedef T type; };

  template <typename Ty>
  struct ValueIncDecLimiter
  {
    typedef typename remove_reference<Ty>::type T;

    struct Native {
      static T limit_max(const T &value, const T &prev) { return value; }
      static T limit_min(const T &value, const T &prev) { return value; }
    };

    template <T vmin, T vmax>
    struct Clamp {
      static T limit_max(const T &value, const T &prev) { return prev == vmax ? vmax : value; }
      static T limit_min(const T &value, const T &prev) { return prev == vmin ? vmin : value; }
    };

    template <T vmin, T vmax>
    struct Repeat {
      static T limit_max(const T &value, const T &prev) { return prev == vmax ? vmin : value; }
      static T limit_min(const T &value, const T &prev) { return prev == vmin ? vmax : value; }
    };
  };

  template <
    typename T,
    typename Limiter = typename ValueIncDecLimiter<T>::Native
  >
  class BaseValueIncDec: public FocusableItem {
    typedef FocusableItem super;
  public:
    explicit BaseValueIncDec(const T &value, const T &increment)
    : value(value), increment(increment) {
    }
    void exec(char command) {
      switch (command)
      {
        case '+':
          value = Limiter::limit_max(value + increment, value);
          break;
        case '-':
          value = Limiter::limit_min(value - increment, value);
          break;
        default:
          super::exec(command);
          break;
      }
    }
  protected:
    T value, increment;
  };

  template <
    typename T,
    typename Limiter = typename ValueIncDecLimiter<T>::Native
  >
  class ValueIncDec: public BaseValueIncDec<T, Limiter> {
    typedef BaseValueIncDec<T, Limiter> super;
  public:
    using super::BaseValueIncDec;
  protected:
    void draw_simple_value(Printer &out) const {
      out.print(this->value);
    }
  };

  template <class T>
  class WithLeadingPlusSign: public T {
    typedef T super;
  public:
    using T::T;
  protected:
    void draw_simple_value(Printer &out) const {
      if (this->value > 0) {
        out.print('+');
      }
      super::draw_simple_value(out);
    }
  };

  template <class T, char chr>
  class WithTrailingChar: public T {
    typedef T super;
  public:
    using T::T;
  protected:
    void draw_simple_value(Printer &out) const {
      super::draw_simple_value(out);
      out.print(chr);
    }
  };

  class BaseValueTimeHHMM {
  protected:
    void draw_time(Printer &out, bool minutes, unsigned short time) const;
  };

  template <
    typename T,
    typename Limiter = typename ValueIncDecLimiter<T>::Native
  >
  class ValueTimeHHMM: public BaseValueIncDec<T, Limiter>, BaseValueTimeHHMM {
    typedef BaseValueIncDec<T, Limiter> super;
  public:
    explicit ValueTimeHHMM(const T &value)
    : super(value, 60) {
    }
    void draw_value(Printer &out) const {
      draw_time(out, this->increment == 1, this->value);
    }
    void exec(char command) {
      switch(command) {
        case '>':
          if (this->increment == 1) {
            super::exec(command);
          } else {
            this->increment = 1;
          }
          break;
        case '.':
          this->increment = 60;
          // fall through
        default:
          super::exec(command);
          break;
      }
    }
  };
}
