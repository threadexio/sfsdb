<div align="center">

<img src="assets/logo.svg" height=250>

</div>

# sfsdb

`S`imple
`F`ile
`S`torage
`D`ata`B`ase

A simple but powerful service to store large and small files.

# What's this?

`sfsdb` is a service that offers the ability to store and retrieve files via a set of commands. It was originally intended for use in webapps to provide a fast and dedicated space for storing files instead of putting them in the database. This option is more beneficial to the developer as it allows for greater flexibility and is only limited by the filesystem it is running under. For example, if your instance is running under `btrfs` you can snapshot all of the stored files with only one command, and theoretically even while the software is running. This flexibility allows the server to be shutdown and spun back up again extremely quickly with very little, if not zero, data loss.

# How does it work?

It uses volumes, which are basically any directory that files can be stored on. These volumes keep a track of the files uploaded by giving them unique identifiers time-based identifiers. Because volumes are just directories, it is possible to change the mount at that directory to alter the purpose of the software. For example, you could mount a `tmpfs` filesystem, with sufficient size, on the volume and have the server act as a file cache instead of long-term storage.

# Building

As simple as:

```
$ make
```

The resulting server and command line client binaries can be found over at `build/src/`
