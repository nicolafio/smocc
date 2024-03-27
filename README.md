# SMOCC

C++ implementation of SMOC, a minigame once implemented in JS+HTML.

## Development setup

_These instructions are for macOS._

* Install [VSCodium] or [VSCode].
    * If using VSCodium, switch the extension registry to Microsoft's as noted in the section
      [Switching extension registry to VScode] under the appendix.
* Install Microsoft's [C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools) extension.
* Install [Homebrew].
* Install [SDL2]: `brew install sdl2`

[VSCodium]: https://vscodium.com
[VSCode]: https://code.visualstudio.com
[Homebrew]: https://brew.sh/
[SDL2]: https://wiki.libsdl.org/SDL2/Installation
[Switching extension registry to VScode]: #switching-extension-registry-to-vscode

## License

See [COPYING].

[![GPLv3 Image]][GPLv3]

Copyright 2024 Nicola Fiori

SMOCC is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

SMOCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program.  If not, see <https://www.gnu.org/licenses/>.

[COPYING]: COPYING
[GPLv3 Image]: https://www.gnu.org/graphics/gplv3-127x51.png
[GPlv3]: http://www.gnu.org/licenses/gpl-3.0.en.html

## Appendix

### Switching extension registry to VScode

Replace content of `~/Library/Application Support/VSCodium/product.json`
with the following:

```json
{
    "extensionsGallery": {
        "serviceUrl": "https://marketplace.visualstudio.com/_apis/public/gallery",
        "itemUrl": "https://marketplace.visualstudio.com/items",
        "cacheUrl": "https://vscode.blob.core.windows.net/gallery/index",
        "controlUrl": ""
    }
}
```
