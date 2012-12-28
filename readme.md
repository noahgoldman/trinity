## Code for Trinity robot 2012-2013

# Maple

Follow the instructions on configuring for linux in order to get everything
working.  On arch you have to create the plugdev group and add yourself to it, 
as it doesn't normally exist. Don't forget to logout after this is done.
Adding yourself to the uucp group can also fix these issues.

In order for our code to work there is a small change that I decided to make to 
the Wire library.  Apply wire.patch in the libraries/ directory with 
"patch -p0 -i wire.patch"

# Compiling

Set the environment var LIB_MAPLE_HOME to the path of libmaple, then run:

make
make install
