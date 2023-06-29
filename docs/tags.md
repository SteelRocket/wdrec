# Tags
See [tagmap.c](/src/tagmap.c) and [tagmap.h](/src/tagmap.h) for implementation.

When using tags make sure to surround the word with `""`.

Tags are split into to 2 sections one for `word` part and other for `replace` part.

Tags are also split based on if they can contain text or sub tags.

## Regular Tags
These tags do not have any condition. Like: `<shift>` `<shift><a></shift>` `<shift>hello</shift>`

## Not Standalone Tags

These tags must contain subtags or text. Like: `<casein>hello<enter></casein>`

## Text only Tags

These tags must contain text only. subtags are not allowed. Like: `<uni>0x03A9</uni>`

## Standalone

These tags should contain no value. Like: `<enter>`

# Word Tags

These are tags given in word part of replace word

## `character tags`

- Is a **standalone** tag.
- Presses and releases the given character in the tag. See [typing vs presses](./info.md#difference-typing-and-presses) for more info
- Example: `<h><i>` or `<ctrl><s></ctrl>`

## `<enter>`

- Is a **standalone** tag.
- Acts as \n for the given word
- Example: `hello<enter>world`

## `<tab>`

- Is a **standalone** tag.
- Acts as \t for the given word
- Example: `hello<tab>world`

## `<casein>`

- Is a **not standalone** tag
- The text inside these tags ignore case sensitivity
- Subtags inside this tag are simply ignored
- Example: `<casein>h</casein>ello` will check for 'hello' and 'Hello' to replace

# Replace Tags

## `<enter>`

- Is a **standalone** tag.
- Acts as \n
- Example: `hello<enter>`

## `<tab>`

- Is a **standalone** tag.
- Acts as \t
- Example: `<tab>hello`

## `<backspace>`

- Is a **standalone** tag
- Deletes the previous character
- Undo is not possible if `<backspace>` backspace is given at first
- Example: `hello<backspace>` or `<backspace><backspace>`

## `<shift>`

- Is a **regular** tag
- Presses shift if only standalone is given
- Holds shift until all text or subtags are finished with it is not standalone
- Undo is not possible if it is used
- Example: `<shift><a></shift>`

## `<ctrl>`

- Is a **regular** tag
- Presses ctrl if only standalone is given
- Holds ctrl until all text or subtags are finished with it is not standalone
- Undo is not possible if it is used
- Example: `<ctrl><a></ctrl>`

## `<uni>`

- Is a **text only** tag
- Types the given unicode based on given decimal or hexadecimal value
- Example: `<uni>0x03A9</uni>` or `<uni>937</uni>`
