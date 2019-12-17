# Automated Test Framework based mobile application (ATF_app)

## Dependencies:
Library                | License
---------------------- | -------------
**Lua libs**           |
liblua5.2              | MIT
json4lua               | MIT
lua-stdlib             | MIT
lua-lpeg               |
**Qt libs**            |
Qt5.9 Network          | LGPL 2.1
Qt5.9 Core             | LGPL 2.1
**Other libs**         |
lpthread               | LGPL
OpenSSL (ssl, crypto)  | OpenSSL License
libxml2                | MIT
mysql-client-5.7       | MIT

For ATF_app usage:
```

```

## Get source code:
**1** Install git
```apt-get install -y git```
**2** Get source code from GitHub
```
$ git clone https://github.com/smartdevicelink/sdl_atf
$ cd sdl_atf
$ git submodule init
$ git submodule update
```

## Compilation:
**1** Install 3d-parties developers libraries
```
apt-get install -y gcc-4.9
apt-get install -y g++-4.9
apt-get install -y cmake
apt-get install -y libssl-dev
apt-get install -y openssl
apt-get install -y liblua5.2-dev
apt-get install -y libxml2-dev
apt-get install -y lua-lpeg-dev
apt-get install -y mysql-client-5.7
```

**2** Install Qt5.9
```
apt-get install -y software-properties-common python-software-properties
add-apt-repository -y ppa:beineri/opt-qt591-xenial
apt-get install -y qt59base
apt-get install -y qt59websockets
```
**3**  Build app
```
cmake <cources_folder_path> -DCMAKE_INSTALL_PREFIX=<destination_path>
make install
```
## Configuration and change logic of application (if needed)
**1** Change app sequence if needed in ```./modules/app```
**2** Set framework configuration in ```./modules/configuration```

## Run:
Run without parameters
