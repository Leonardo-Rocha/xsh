# ysh

ysh is a ncurses based implementation of a simple linux shell.

## Dependencies

#### Readlines

To maintain a history of commands, we're using [readlines/history](https://tiswww.case.edu/php/chet/readline/rltop.html).

Install by typing:

```
sudo apt-get install libreadline-dev
```

#### Curses

To create a beautiful text-interface we're using [ncurses](https://invisible-island.net/ncurses/).

Install by typing:

```
sudo apt-get install libncurses5-dev libncursesw5-dev
```

## How to run

For the first use 'make' to compile and run the program. For further uses 'make run'.

## Commands

### Arrow Up/Down

Use arrow up/down to navigate through command history.
