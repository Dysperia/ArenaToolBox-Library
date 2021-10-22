ArenaToolBox Library V1.0
==============

# Summary

ArenaToolBox Library is part of the ArenaToolBox project which want to provide translators and modders various tools for 
creating, editing and update the different The Elder Scrolls 1: Arena file formats.
BSATool provides support for exploring, editing and creating Bethesda Arena BSA. Some other game of Bethesda of the same 
period can also be edited with this tool.

ArenaToolBox Library is not a tool but contains classes used to build the tools themselves. Among all the code 
is a compression utility than can compress and uncompress most of the assets.

For those interested, there is a compression algorithm associated with the compression flag 0x08 (the deflate 
algorithm). As of now (2021) ArenaToolBox could be the only project to be fully compatible with the one arena use. 
I explain: the other project concerning arena have some compression/uncompression algorithm and they works for all 
the assets. However, the huffman tree has a build in reset when the frequency of the root node is too high. Of course, 
feel free to use my code to improve yours if you find something useful. I tried to optimize a lot the algorithms to have 
some speed while compressing, while implementing the full mirror of the uncompression algorithm arena uses. If you look
at the test ressources, you will also find some test files, including one (deflateWorstCase) able to trigger the tree reset (Arena has no
file able to do that as far as I know). I made it by generating a file with few pattern repeating (at least in the 
sliding window).

ArenaToolBox was created by David Aussourd alias Dysperia (softwatermermaid@hotmail.fr) with Qt 5.

# ArenaToolBox parts :

* [ArenaToolBox Library](https://github.com/Dysperia/ArenaToolBox-Library "ArenaToolBox Library") (common code used by different tools)
* [BSATool](https://github.com/Dysperia/ArenaToolBox-BSATool "BSATool") (BSA archive explorer and manager)
* [TemplateEditor](https://github.com/Dysperia/ArenaToolBox-TemplateEditor "TemplateEditor") (TEMPLATE.DAT editor)