# pimii - The Vision
pimii is programming language intended for education and research. This project contains a compiler along with a LISP-like virtual machine (executing SECD code). It uses a SmallTalk-80 like syntax as input and is therefore very flexible.

To use pimii without compiling, you can download the /deploy folder which contains a more or less up-to-date version of the vm.

The language is pure functional and has no built in control structures or keywords. Therefore it could be completely translated into a foreign (human) language.

To use this project you have to clone the repository and install the Qt-Creator (comes with the Qt SDK). You'll then be able to open this project in the Qt Creator.

# Language

## Types
pimii knows:

 - Numbers (range depends on your wordsize of your machine). The default startup script reveals the wordsize ;-)
 - Decimal numbers (floats)
 - Symbols (not in a LISP sense). Everything starting with a hash is a symbol and can be thought of a self representing constant. Examples are #TRUE and #FALSE which are used to represent results of boolean functions.
 - Cells: A cell is a pair of two values (car, cdr). It can be used to build linked lists. Being a LISP descendand, nearly everything in pimii is a list, including functions.

## Syntax

 - TODO: Learn from some examples in deploy/examples/example.pi and /deploy/lib/pimii.pi (You should launch pimii which provides syntax highlighting :-)

