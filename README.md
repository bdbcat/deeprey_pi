
Linux Build
Prerequisites...

1. cmake c++ development stack
2. wxWidgets development files, installed in standard location.
3. OpenCPN installed and tested in standard method for platform, typically installed from repo, or built locally.

Building the plugin...
$ cd deeprey_pi

$ git submodule init

$ git submodule update 

$ mkdir build

$ cd build

$ cmake ..

The build artifact is normally a tarball, typically like: "deeprey_pi/build/DEEPREY-0.1.0.0+2410061344._debian-x86_64-12-x86_64.tar.gz"
In production, this tarball is stored online, and an entry is added to the OpenCPN catalog to allow download and installation of the plugin.

For production this build and upload step is managed by a CI/CD process.

For local plugin building and testing, OpenCPN may directly "import" a locally built plugin tarball.

To do this, a special configuration setting in OpenCPN must be enabled.

a.  Stop OpenCPN, if running.

b.  edit the OpenCPN configuration file, "~/.opencpn/opencpn.conf"

c.  Add the following key/value to the "PlugIns" section of the config file:

[PlugIns]

CatalogExpert=1

d.  Restart OpenCPN

e.  Navigate to Settings->Plugins.  Touch the "Import" button, select the tarball, and proceed.

f.  The imported plugin, if accepted, will now appear in the Plugin Manager list, ready for activation.

Windows Build
TODO
