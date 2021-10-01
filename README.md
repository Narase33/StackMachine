# StackMachine
## The repository
A stackmachine (hobby project)

StackMachine is a hobby project I created to get myself into VMs and interpreters and to stay in touch with the C++ programming language, which I dont use during my normal job as software engineer.

There is no specific goal to this project except for keeping me busy with C++ and learning new techniques in software development in general.

If you found my repository and you have questions or suggestions about my project or code, feel free to contact me. I'm far away from beeing a professional C++ developer as I'm 99% self-taught, so excuse any rookie mistakes I might have done. I'd be happy to help or learn from others.

## The project
The project implements a stackmachine (https://en.wikipedia.org/wiki/Stack_machine). It doesn't interpret on the pure text like Python, but creates a bytecode in the first step, so it's more like Java. The interpreted language will look C'ish.


The language it interprets grows with the project in a feedback loop, it's not fixed, it's not a waterfall model.

This means when I think of a new feature I'd like to have in my language, I look how it can be implemented.
While implementing the new feature I might see that it's hard for the interpreter to distinguish it from another feature or it takes too much computing power to get it right or it creates too much bytecode that cannot be optimized away.
In this case the feature changes in order to have a syntax thats better/faster to interpret.


Since the project is far away from beeing finished there is no plug&play way of actually using it at this time. You can start the project and see that the UnitTests are successful, but thats it.

UnitTests are currently also the best way to see what features the language already has what it looks like.

This project has the goal to be easy to build.
I won't use any libs that need to be installed (except for the STL, ofc), even if that means to reinvent the wheel from time to time.
I won't use any platform specific functions or any other stuff that might trouble the building process.
There is a cmake file that is guaranteed to work as long as you have a compiler that is able to use C++17.
