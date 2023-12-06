# Vulkan-BuildingBlocks

If you are reading this, it is a very EARLY Work In Progress...

The Vulkan Building Blocks (VBB) are C++ classes full of reusable and customizable Vulkan components for 3D and GPU compute applications. These classes are intended to streamline Vulkan development across desktop and mobile platforms. They are tested regularly on Windows, macOS, Linux, iOS, and (TBD: Android devices). A critical design goal is to not be an "abstraction" so much as pre-written Vulkan modules that are easily extendable and do not enforce application design policy. There are no new enumerants, error codes or data types, everything is just Vulkan with frequently used functionality pre-packaged into easy to use and modify C++ classes.

The only dependency of the VBB classes is the VulkanSDK from LunarG. VBB makes heavy use of two essential "libraries" for Vulkan development, the Vulkan Memory Allocator (VMA), and the Volk extension loader. These tools dramatically simplify your Vulkan code and are best integrated from the start of any project, and not added later as an after-thought. The VBB makes use of both throughout. The examples also use the glm mathematics library and SDL.

Code Conventions

All core classes begin with VBBName, where Name is the CamelCase name of the class

Namespaces are not used. A short namespace like vbb:: is more likely to collide with another library than class names prefixed with VBB. VBBInstance, VBBDevice, etc. If you absolutely need namespaces feel free to create a fork and stick them in yourself.

There are no VBB specific enumerants or data types that mirror or mask Vulkan enums. I realize there is often some desire to make Vulkan more type-safe for C++, but given Vulkans low level nature, and my desire to not "abstract away" Vulkan or it's performance advantages, I'm decideing against it. I highly advise the use of the Khronos Vulkan Validation layer to help catch silly errors as you develop rather than create whole new API to learn that rides on top of Vulkan. If you are using the VBB, you are writing Vulkan code. VBB is not an engine, it's just Vulkan. Another design goal is that the VBB components can be dropped into an existing Vulkan program easily, or VBB classes can be easily modified by a child class, or even replaced completely with other C/C++ code. This is an important design goal as VBB's aim is to help you get going quickly, but not handcuff you if you need to start doing very advanced or performance critical rendering work.

Member functions begin with a lower case letter.
Member variables start with m_ and are camelCase

Other deisgn philosophys at the risk of repeating myself...

This is not meant to be a cutting-edge Vulkan demo framework. It is meant to be useful for learning Vulkan, but also for shipping Vulkan applications on the widest possible range of hardware and drivers. The VBB stops short purposefully of making use of a lot of extensions (use as few as possible), or advanced rendering techniques, but should make an effort to "stay out of the way", should those techniques need to be added later.

The C++ code is also meant to be very simple and straighforward, and not require a great deal of C++ expertese to read and understand. C programmers with a minimual understanding of C++ should be able to navigate VBB easily. Beginners to Vulkan are often also beginners to C/C++, and beginners to programming in general. Never-the-less, as stated previously, the framework should also be suitable for professional useage. Professional in this sense does necessarily not equal AAA game engines, but rather the vastly unmet need for simpler 3D rendering applicitons that don't have thousands of pipelines and need maximum paralllelization to just be usable. There is nothing keeping anyone from extending VBB with their super-awesome template wielding modern C++ 20 or better puissance.

Finally, there is no windowing, or display functionality. You can drop the VBB Canvas class (VBBCanvas) into Win32 native, Objective C, SDL, glfw, Qt, MFC, Lazarus, Turbo Pascal, Fortran 77... okay, maybe not, but you get my drift.

Oh, I lied, there is a QtVulkanWindow class, maybe I'll take that out, but I sure find it handy. Still, there's no code in VBB to actually display a window, accept input, etc. Use VBB with any applicaton framework you like, as long as it allows you to include and use C++ classes. If I've done my job right, there should be very very little platform specific #defines, it should all just be Vulkan and C++.

Oh, you are on your own for making a surface... maybe we'll add some helpers for that too. If you're using Qt... your're welcome already for the above mentioned class.

TBD: Samples that work on all platforms, and use several differetnt application frameworks.

I have spoken. 

Feel free to go write your own Vulkan helper library... and if you are capable of that why wre your reading this and giving me a hard time about it? 

Richard S. Wright Jr.
richard@lunarg.com
