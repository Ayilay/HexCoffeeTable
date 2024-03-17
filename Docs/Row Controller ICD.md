# Interface Control Document for Row Controller

## Terminology

| Term             | Explanation |
| ---------------- | ----------- |
| Row Controller   | tbd         |
| Cell             | tbd         |
| Table Controller | tbd         |

## Row-Column Convention

The coffee consists of tessalating hexagons. For representing the position of each cell, we assume the coffee table is a rectangular grid with 13 rows and 8 columns.

From the Top-View, it looks as follows:

| Row/Col | C0  | C1  | ... | C8  |
| ------- | --- | --- | --- | --- |
| R0      |     |     |     |     |
| R1      |     |     |     |     |
| ...     |     |     |     |     |
| R13     |     |     |     |     |

### Control of each cell

For every row, the cell at column 0 contains a microcontroller that behaves as an I2C slave. This microcontroller is referred to as the Row Controller.

Every row controller (13 total) has a unique I2C address, and all row controllers share the I2C bus.

The row controller can individually set the color state of any hexagon in its row. It can also read the individual state of each hexagon in its row (i.e. is an object placed on it?). 

The hexagons on each row (except the one at column 0) do not have a microcontroller.

The row controllers are "dumb". They receive the RGB color data, and forward it to the appropriate cells, and do not support any computation beyond that.

The I2C Master is a separate microcontroller with enhanced computational abilities. It is referred to as the Table Controller. The Table Controller computes the table lighting patterns and animations, and sets the color state of the table cells by talking to the row controllers.

## Write Data

### Basic Format

| Byte Offset | nBytes | Data Type | Description                   |
| ----------- | ------ | --------- | ----------------------------- |
| 0           | 1      | U8        | Op Code                       |
| 1           | N      | -         | Payload. N depends on Op Code |

### Data Types

Following the C-style of data types, the number (xx) in the Data Type designator denotes the bit-width of the type

| Data Type designator | Example(s) | Description      |
| -------------------- | ---------- | ---------------- |
| Uxx                  | U8, U32    | Unsigned integer |
| Sxx                  | S8, S16    | Signed integer   |
| RGB16                | -          | 16-bit RGB value |
| RGB24                | -          | 24-bit RGB value |

### RGB Types

The RGBxx types are encoded as follows, where bit 0 is the least-significant bit (right-most bit)

#### RGB24 (3-bytes)

This mode supports 16,777,216 unique color combinations.

| Bits  | Numeric Range | Description |
| ----- | ------------- | ----------- |
| 16-23 | 0-255         | Red         |
| 8-15  | 0-255         | Green       |
| 0-7   | 0-255         | Blue        |

#### RGB16 (2-bytes)

This mode supports 65,536 unique color combinations.

The advantage of 2-byte colors is that if we don't want to use the whole 16million color range of 3-byte colors, we can save 8 bytes per transfer when setting the entire Row Colors for faster frame rates, and still get very decent color resolution.

| Bits  | Numeric Range | Description |
| ----- | ------------- | ----------- |
| 11-15 | 32            | Red         |
| 5-10  | 64            | Green       |
| 0-4   | 32            | Blue        |

### Op Codes

The op-codes follow a 7-4 Hamming coding. We can thus support up to 16 total Op Codes that are guaranteed 3-bit-flips apart.

| Op Code | Name         | Summary                                                   |
| ------- | ------------ | --------------------------------------------------------- |
| 0x01    | SetCell3B    | Set color of single cell; 3-byte color                    |
| 0x13    | SetRow3B     | Set color of entire row; 3-byte color                     |
| 0x1D    | SetCell2B    | Set color of single cell; 2-byte color                    |
| 0x25    | SetRow2B     | Set color of entire row; 2-byte color                     |
| 0x2A    | ConfigUpdate | Configure when the cell controller performs an LED update |
| 0x36    | Reserved     |                                                           |
| 0x39    | Reserved     |                                                           |
| 0x46    | Reserved     |                                                           |
| 0x49    | Reserved     |                                                           |
| 0x55    | Reserved     |                                                           |

### Packet Descriptions

#### SetCell3B

Set the color of a single cell in this row.

| Byte Offset | nBytes | Data Type | Description                 |
| ----------- | ------ | --------- | --------------------------- |
| 0           | 1      | U8        | Op Code (see Op Code table) |
| 1           | 1      | U8        | Column of Cell to set       |
| 2           | 3      | RGB24     | RGB Value                   |

#### SetRow3B

Set the color of all cells in this row. We assume the row always has 8 cells.

| Byte Offset | nBytes | Data Type      | Description                 |
| ----------- | ------ | -------------- | --------------------------- |
| 0           | 1      | U8             | Op Code (see Op Code table) |
| 1           | 24     | Array of RGB24 | RGB Values of Cells 1-8     |

### ConfigUpdate

Sometimes, we want the row controller to update its LEDs immediately upon receiving the color data. When writing individual cells however, it may be better to "write multiple cells", and then ask the row controller to update them all at once.

| Byte Offset | nBytes | Data Type | Description                 |
| ----------- | ------ | --------- | --------------------------- |
| 0           | 1      | U8        | Op Code (see Op Code table) |
| 1           | 1      | U8        | Action                      |

The Action argument is as follows:

| Value | Description                                                                                   |
| ----- | --------------------------------------------------------------------------------------------- |
| 0     | Set mode to Update LEDs Immediately                                                           |
| 1     | Set mode to not update LEDs. The LEDs will only be updated if the 'Update Now' action is sent |
| 2     | Update all LEDs Now                                                                           |

## Read Data

When the Table Controller (I2C Master) performs an I2C Read to any of the Row Controllers, the Row Controller always returns the same data format.

We do not support Write-then-Read transfers like other traditional I2C devices.

### Read Data Format

| Byte Offset | nBytes | Data Type | Description        |
| ----------- | ------ | --------- | ------------------ |
| 0           | 1      | U8        | Row State Bit Mask |

The row state encodes the state of columns 0-7 as follows:

- Bit x = 0; The column does not detect anything on it

- Bit x = 1; The column detects the presence of an object

Column x always corresponds to Bit x. Note that Bit 0 is the LSb and Bit 7 is the MSb
