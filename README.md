# CSED353(Computer Network)

## Objective
Today, Internet access is considered a basic human right.\
Nearly every computer, mobile device, and even wearable that we interact with on a daily basis are connected over the network. We cannot imagine a computer without a network; we cannot imagine our life without a networked computer.

In this light, this course is designed to help the students learn from mainly three dimensions as follows:
1. Lectures. 
- We will discuss basic principles and topics of fundamental concepts concerning the technology and architecture of the Internet. Topics to be covered mainly in this course will include IP, TCP/UDP, and Internet applications. 
- The lectures will also feature advanced topics, namely "network beyond connectivity", where we will taste modern case studies that utilize the network as a physical sensor, a privacy defender, a social facilitator, and so on.

2. Brand-new Assignments. 
- As of 2022, this course will feature whole re-designed assignments. These assignments will guide the students to develop hands-on technical experiences by building the key mechanisms of the protocols making modern computers talk to each other.

3. Global Industry. 
- A specially invited POSTECH graduate (now working for a tech giant in Silicon Valley) will offer a guest lecture on the cutting-edge global networking industry.
- This guest lecture is to broaden the eyes of our fellow POSTECH juniors, to depict a career path that may be yours one day, and to give a crisp purpose for learning networks.

## Professor
Name: Inseok Hwang\
Homepage: HTTPS://WWW.HIS-LAB.ORG

## Grading Policy
Mid-term 20%\
Final 25%\
Programming assignments 35%\
Written assignments & quizzes 10%\
Interactivity and participation 10%

## References
- Computer Networking – A Top-Down Approach by J. F. Kurose and K. W. Ross, 8th edition. (Global edition) Pearson.
- Additional materials will be provided if necessary

## Lecture Schedule
- Overview of computer network 
- Understanding of layers
	- Application layer
 	- Transport layer 
 	- Network layer
 	- Data link layer
- Special guest lecture on networking: to be offered by a POSTECH graduate now working for a major tech giant in Silicon Valley
- Advanced topics on networking: computer networking as a physical sensor, a privacy defender, and a social facilitator

---
## CSED353 Lab Assignments

For build prereqs, see [the VM setup instructions](http://tomahawk.postech.ac.kr/csed353/assignments/vm/virtualbox).

## Sponge quickstart

To set up your build directory:

	$ mkdir -p <path/to/sponge>/build
	$ cd <path/to/sponge>/build
	$ cmake ..

**Note:** all further commands listed below should be run from the `build` dir.

To build:

    $ make

You can use the `-j` switch to build in parallel, e.g.,

    $ make -j$(nproc)

To test (after building; make sure you've got the [build prereqs](http://tomahawk.postech.ac.kr/csed353/assignments/vm/virtualbox) installed!)

    $ make check_labN *(replacing N with a checkpoint number)*

The first time you run `make check_lab...`, it will run `sudo` to configure two
[TUN](https://www.kernel.org/doc/Documentation/networking/tuntap.txt) devices for use during
testing.

### build options

You can specify a different compiler when you run cmake:

    $ CC=clang CXX=clang++ cmake ..

You can also specify `CLANG_TIDY=` or `CLANG_FORMAT=` (see "other useful targets", below).

Sponge's build system supports several different build targets. By default, cmake chooses the `Release`
target, which enables the usual optimizations. The `Debug` target enables debugging and reduces the
level of optimization. To choose the `Debug` target:

    $ cmake .. -DCMAKE_BUILD_TYPE=Debug

The following targets are supported:

- `Release` - optimizations
- `Debug` - debug symbols and `-Og`
- `RelASan` - release build with [ASan](https://en.wikipedia.org/wiki/AddressSanitizer) and
  [UBSan](https://developers.redhat.com/blog/2014/10/16/gcc-undefined-behavior-sanitizer-ubsan/)
- `RelTSan` - release build with
  [ThreadSan](https://developer.mozilla.org/en-US/docs/Mozilla/Projects/Thread_Sanitizer)
- `DebugASan` - debug build with ASan and UBSan
- `DebugTSan` - debug build with ThreadSan

Of course, you can combine all of the above, e.g.,

    $ CLANG_TIDY=clang-tidy-6.0 CXX=clang++-6.0 .. -DCMAKE_BUILD_TYPE=Debug

**Note:** if you want to change `CC`, `CXX`, `CLANG_TIDY`, or `CLANG_FORMAT`, you need to remove
`build/CMakeCache.txt` and re-run cmake. (This isn't necessary for `CMAKE_BUILD_TYPE`.)

### other useful targets

To generate documentation (you'll need `doxygen`; output will be in `build/doc/`):

    $ make doc

To format (you'll need `clang-format`):

    $ make format

To see all available targets,

    $ make help
