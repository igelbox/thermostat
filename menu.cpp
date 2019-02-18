#include "menu.hpp"

namespace menu {
  AbstractItem *focused = nullptr;

  void AbstractItem::draw_value(Printer &out) const {
    if (!out.blink) {
      draw_simple_value(out);
    }
  }
  void AbstractItem::draw_screen(Printer &out, uchar lines_count) const {}
  void AbstractItem::exec(char command) {}
  void AbstractItem::draw_simple_value(Printer &out) const {}


  void FocusableItem::draw_screen(Printer &out, uchar lines_count) const {
    previous->draw_screen(out, lines_count);
  }
  void FocusableItem::exec(char command) {
    switch (command)
    {
      case '>':
        focused = previous;
        break;
      case '.':
        previous = focused;
        focused = this;
        break;
      default:
        super::exec(command);
        break;
    }
  }

  const char *List::BACK_TEXT = "Back";
  List::List(List::Item *items, uchar count)
    : items(items), count(count), max_index(count - 1), cursor(0), offset(0) {
  }
  void List::draw_screen(Printer &out, uchar lines_count) const {
    bool blink = out.blink;
    for (auto i = 0; i < lines_count; ++i) {
      auto index = i + offset;
      char pointer = ' ';
      if ((index == cursor) && (focused == this)) {
        pointer = index == count ? '<' : '>';
      }
      out.print(pointer);
      if (index == count) {
        out.print(BACK_TEXT);
      } else {
        auto &item = items[index];
        out.print(item.label);
        out.blink = blink && (focused == &item.item);
        item.item.draw_value(out);
      }
      out.println();
    }
  }
  void List::exec(char command) {
    switch (command)
    {
      case '-':
        if (cursor != 0) {
          offset = --cursor;
        }
        break;
      case '+':
        if (cursor != max_index) {
          offset = cursor++;
        }
        break;
      case '>':
        if (cursor != count) {
          items[cursor].item.exec('.');
          break;
        }
      case '.':
        max_index = count;
        // fall through
      default:
        super::exec(command);
        break;
    }
  }

  void print_lz2(struct Printer &out, uchar v) {
    if (v < 10) {
      out.print('0');
    }
    out.print(v);
  }

  void BaseValueTimeHHMM::draw_time(Printer &out, bool minutes, unsigned short value) const {
    auto blink = out.blink;
    if (blink && !minutes) {
      out.print("  ");
    } else {
      print_lz2(out, value / 60);
    }
    out.print(':');
    if (blink && minutes) {
      out.print("  ");
    } else {
      print_lz2(out, value % 60);
    }
  }
}
