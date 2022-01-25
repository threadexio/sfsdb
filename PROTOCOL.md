# Network protocol spec

| Version |
| :-----: |
|    1    |

This is a full description of the communication protocol used for the server and client.

## Networking

The protocol can be implemented on any kind of data transfer mechanism, not just over TCP. It should be possible to use the protocol over anything, like unix sockets and theoretically even radio.

It was designed with these goals in mind:

-   Scalability
-   Easy parsing
-   Speed
-   Size

If parsed correctly the protocol is binary-safe, meaning you can transport quite literally anything with it.

**NOTE**: Currently the protocol does not implement any kind of way to check for the validity of data. That must be done by the server.

## Structure

The protocol consits of some basic types that are used to encode information.

| Type ID |    Type    |     Fields      | Field type  | Description                             |
| :-----: | :--------: | :-------------: | :---------: | :-------------------------------------- |
|    0    | `INVALID`  |     `None`      |   `None`    | Indicate the absense of data            |
|    1    |  `HEADER`  |    `command`    |  `uint16`   | Encodes the command the client issued   |
|    2    |  `ERROR`   |      `msg`      | `char[256]` | A small string to encode error messages |
|    3    | `SMALLINT` |      `val`      |  `uint16`   | A 16-bit integer                        |
|    4    | `INTEGER`  |      `val`      |  `uint32`   | A 32-bit integer                        |
|    5    |  `STRING`  |      `str`      |  `string`   | A variable-length string                |
|    6    | `BIGDATA`  | [see](#bigdata) |  `byte[]`   | A type that encodes binary data         |

As this is a binary protocol, all numeric types must be converted to network endianess before transmission.

All types are prepended with another header, which is the **data header**. The structure of that header is:

|  Field   | Field type | Description                                          |
| :------: | :--------: | :--------------------------------------------------- |
|  `type`  |  `uint16`  | Encodes the type number of information that follows. |
| `length` |  `uint32`  | Encodes the length of the information that follows.  |

The order that the fields are encoded is: `type` -> `length`

``
          type    length
           \/       \/
00000000: 0002 | 0000 000f                           ......
```

For example, the above would be the data header for an error message (`type = 2`) with a length of 15.

---

### INVALID

`INVALID` is a special type that contains no information and consists of only the data header, with:

-   `type = 0`
-   `length = 0`

The byte representation is:

```
00000000: 0000 0000 0000                           ......
```

---

### HEADER

The `HEADER` type is the start of every network message and it encodes the length of the whole message minus the length of itself and the command or some other status to be ran.

**NOTE:** Depending on the sender, the `command` field can be used for 2 things:

-   Command number (if sender is the client)
-   Status code (is sender is the server)

Byte representation for the header of a message that is 20 bytes long is:

```
00000000: 0001 0000 0014 0000                      ........
```

---

### ERROR

The `ERROR` type is a small string that can be used to communicate possible error messages. It is limited at 256 chars max.

Byte representation for `"Test error message"`:

```
00000000: 0002 0000 0012 5465 7374 2065 7272 6f72  ......Test error
00000010: 206d 6573 7361 6765                       message
```

---

### SMALLINT

A numeric type that encodes `uint16`. Must be converted to network endianess.

Byte representation for `1234`:

```
00000000: 0003 0000 0002 04d2                      ........
```

---

### INTEGER

A numeric type that encodes `uint32`. Must be converterd to network endianess.

Byte representation for `5678`:

```
00000000: 0004 0000 0004 0000 162e                 ..........
```

---

### STRING

Byte representation for `"Test string"`:

```
00000000: 0005 0000 000b 5465 7374 2073 7472 696e  ......Test strin
00000010: 67                                       g
```

---

### BIGDATA

A type that encodes large blobs of binary data. It is preferred over `STRING` even for large strings as it might be handled differently and more efficiently at the other endpoint. For example, a `STRING` might be read in one go and kept into memory for processing while `BIGDATA` might be read with multiple read calls and processed in chunks, thus leading to lower memory usage.

This type does not have additional fields for information as it expects the caller to write that information themselves, it only encodes the length of that information.

For this reason, the caller is also responsible for reading the data from the other endpoint when the parser encounters this type.

Byte representation for a buffer with `9999` bytes:

```
00000000: 0006 0000 270f                           ....'.
... (9999 bytes)
```
