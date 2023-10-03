# Command line interface

The command line interface is documented in the `-h, --help` flags. CLI uses the http rest api to communicate between wdrec server and wdrec command. 

# Http Rest Api

1. [/version](./api/version.md)
2. [/add](./api/add.md)
3. [/remove](./api/remove.md)
4. [/list](./api/list.md)
5. [/config](./api/config.md)

# Typing System

## Replacing
The text will be replaced as soon as you type it.

## Undo
Undo is possible by pressing backspace after the replace. But undo is disabled if tags like `<ctrl>` are used.


# Replace Word Structure
Each replace word has the following additional attributes.

## Id
Ids are auto assigned when a word is added. we can remove or config a replace word by id using `--id` option.

## Enabled
This attibutes states whether the replace word is active or not.

We can enable or disable a replword word by using the config subcommand

## Temporary
This attibutes states whether the replace word should be saved perminantly.

# Tag syntax

## Tags

See [tags.md](tags.md) for all the tags available.
<hr>

wdrec allows the usage of tag syntax `<tags></tags>` within words.

For example we can do,
```
wdrec add "hello<enter>" world
```
which will only replace hello after enter.  

Another example is,
```
wdrec add "<casein>hello</casein>" word
```
which will ignore the case on the typed hello.

We can of course use tags in replace part also
```
wdrec add hello "word<enter>"
```
Which will type word and then enter

## How to use `<` and `>`
To use the characters `<` and `>` in the replace word do `<<` and `>>` which will be converted into `<` and `>`. So for example `<<shift>>` will type `<shift>` instead of pressing shift 

## Difference typing and presses

In the replace part we can use `<a><b><c>` tags to press the a, b, c buttons. But this is not the same as just typing "abc". The tag characters will be presses and released like normal keys and so they will be affected by caps lock, ctrl, shift, etc. But in regular text the "abc" will not be affected by these keys.

So, `<ctrl><a></ctrl>` will select all text in a text editor but `<ctrl>a</ctrl>` will simply type 'a'
