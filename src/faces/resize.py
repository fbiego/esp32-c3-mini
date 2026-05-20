from PIL import Image, ImageFont, ImageDraw
from os.path import dirname, join
import io
from os import path
import sys

known = [
    0x00, # Hour no
    0x01, # Minute no
    0x02, # Date no
    0x03, # Month no
    0x06, # Weekday label
    0x07, # Year no -
    0x08, # AM/PM label
    0x09, # Image/Icon
    0x0A, # Connection label
    0x0B, # Battery no
    0x0C, # Sleep label
    0x0D, # Analog hands
    0x0E, # Steps no
    0x0F, # Calories no
    0x10, # Heart Rate no
    0x11, # SP02 no
    0x13, # Sleep no
    0x14, # Distance no
    0x16, # Weather no
    0x17, # Weather label
    0x19, # Solid color
    0x1B, # Seconds, made static icon
    0xFD  # Animation
]

advanced = [
    0x1B,   # seconds
    0xFA,   # Weather no+ label
    0xFB    # clickable
]


def bitCheck(value, check):
    return (value & check) == check

def pad_byte_array(input_array, expected_size, pad_byte = 0x00):
    if expected_size < 0:
        return input_array

    current_size = len(input_array)

    if current_size >= expected_size:
        # If the current size is greater than or equal to the expected size, return the original array
        return input_array[:expected_size]
    else:
        # If the current size is smaller, create a new array and pad it with the specified byte
        padded_array = bytearray(input_array)
        padded_array.extend([pad_byte] * (expected_size - current_size))
        return padded_array

def create_image(width, height, data, cSize, nW, nH, transparent = False):
    # Create a new image with the original size
    image = Image.new('RGB', (width, height))

    # Iterate through the data list and set the pixel values
    index = 0
    for y in range(height):
        for x in range(width):
            # Read two consecutive bytes as a single RGB565 value
            rgb565 = (data[index + 1] << 8) | data[index]
            r = (rgb565 >> 11) & 0x1F
            g = (rgb565 >> 5) & 0x3F
            b = rgb565 & 0x1F
            r = (r << 3) | (r >> 2)
            g = (g << 2) | (g >> 4)
            b = (b << 3) | (b >> 2)
            image.putpixel((x, y), (r, g, b))
            index += 2

    # image2 = image.resize((int(width / SF), int(height / SF))).quantize(colors=cSize)
    image2 = image.resize((nW, nH)).quantize(colors=cSize) #resize the image to the new size and quantize
    indexed_image = image2.convert("P", palette=Image.ADAPTIVE) # create an indexed image , palette=Image.ADAPTIVE
    # # Retrieve the color table (palette)
    color_table = indexed_image.getpalette()

    rgb565_color_table = bytearray()

    for i in range(0, len(color_table), 3):
        # Extract the RGB components from RGB24
        r = color_table[i] >> 3
        g = color_table[i + 1] >> 2
        b = color_table[i + 2] >> 3

        # Calculate luminance
        luminance = 0.2126 * color_table[i] + 0.7152 * color_table[i + 1] + 0.0722 * color_table[i + 2]

        # Normalize luminance to a range of 0 to 100
        brightness_percentage = (luminance / 255) * 100

        # Check if brightness is above 50%
        if brightness_percentage < 0 and transparent:
            r = 0
            g = 0
            b = 0

        # Combine the components into RGB565 format (16 bits)
        rgb565_color = ((r << 11) | (g << 5) | b).to_bytes(2, byteorder='big')

        # Append the RGB565 color to the new color table
        rgb565_color_table.append(pint(rgb565_color[1]))
        rgb565_color_table.append(pint(rgb565_color[0]))
       
    # print("color size swap: ", len(rgb565_color_table))
    # # Retrieve the indexes as a list
    indexes = list(indexed_image.getdata())

    return rgb565_color_table, indexes

def create_text(width, height, text, nW, nH, dts):

    bg = (0, 0, 0)
    fg = (255,255,255)

    offset = dts[5] & 0x1F
    if bitCheck(dts[5], 0x20):
        offset = offset * -1
    
    size = dts[4] & 0x3F
    
    image = Image.new('RGB', (width, height), bg)

    Im = ImageDraw.Draw(image)
    fontfile = join(dirname(__file__), "NSTL-R.ttf")
    mf = ImageFont.truetype(fontfile, size)
    # Add Text to an image
    Im.text((0, offset), text, fg, font=mf)


    image2 = image.resize((nW, nH)).quantize(colors=255) #resize the image to the new size and quantize
    indexed_image = image2.convert("P", palette=Image.ADAPTIVE) # create an indexed image , palette=Image.ADAPTIVE
    # # Retrieve the color table (palette)
    color_table = indexed_image.getpalette()

    rgb565_color_table = bytearray()

    for i in range(0, len(color_table), 3):
        # Extract the RGB components from RGB24
        r = color_table[i] >> 3
        g = color_table[i + 1] >> 2
        b = color_table[i + 2] >> 3

        # Calculate luminance
        luminance = 0.2126 * color_table[i] + 0.7152 * color_table[i + 1] + 0.0722 * color_table[i + 2]

        # Normalize luminance to a range of 0 to 100
        brightness_percentage = (luminance / 255) * 100

        # Check if brightness is above 50%
        if brightness_percentage < 10:
            r = 0
            g = 0
            b = 0

        # Combine the components into RGB565 format (16 bits)
        rgb565_color = ((r << 11) | (g << 5) | b).to_bytes(2, byteorder='big')

        # Append the RGB565 color to the new color table
        rgb565_color_table.append(pint(rgb565_color[1]))
        rgb565_color_table.append(pint(rgb565_color[0]))
       
    # print("color size swap: ", len(rgb565_color_table))
    # # Retrieve the indexes as a list
    indexes = list(indexed_image.getdata())
    return rgb565_color_table, indexes

def pint(byte_value):
    # Ensure byte_value is in the range -128 to 127
    byte_value = byte_value & 0xFF
    return byte_value

def resize(file, original, target, location, compatibility = False):
    SF = original/target
    size = file[0]

    cmd = compatibility #compatibility mode (False default)

    new = bytearray()
    new.append(size)
    new.append(0)
    new.append(0)
    new.append(0)

    for i in range(size):
        for y in range(20):
            new.append(0)
    rcs = {}

    for x in range(size):
        id = pint(file[(x * 20) + 4])
        xOff = ((pint(file[(x * 20) + 9]) * 256) + pint(file[(x * 20) + 8]))
        yOff = ((pint(file[(x * 20) + 11]) * 256) + pint(file[(x * 20) + 10]))
        xSz = ((pint(file[(x * 20) + 13]) * 256) + pint(file[(x * 20) + 12]))
        ySz = ((pint(file[(x * 20) + 15]) * 256) + pint(file[(x * 20) + 14]))

        nR = (file[(x * 20) + 5] & 0x7F) #number of grouped elements per item
        group = (file[(x * 20) + 5] & 0x80)

        clt = (pint(file[(x * 20) + 19]) * 256 * 256 * 256) + (pint(file[(x * 20) + 18]) * 256 * 256) + (pint(file[(x * 20) + 17]) * 256) + pint(file[(x * 20) + 16])
        dat = (pint(file[(x * 20) + 23]) * 256 * 256 * 256) + (pint(file[(x * 20) + 22]) * 256 * 256) + (pint(file[(x * 20) + 21]) * 256) + pint(file[(x * 20) + 20])
        print("ID:", "{:02X}".format(id)," ", "No:", "{:02X}".format(nR), xOff, " ", yOff, " ", xSz, " ", ySz, " ", clt, " ", dat)

        lcs = bitCheck(pint(file[(x * 20) + 20]), 0x80)


        cZ1 = (dat - clt)

        if cZ1 >= 512:
            cZ1 = 510
        cSz = (cZ1 // 2) % 256
        output = []

        nXoff = int(xOff / SF)
        nYoff = int(yOff / SF)

        if group != 0x80:
            nR = 1
        if nR == 0:
            nR = 1

        nXsz = int(xSz / SF)
        nYsz = int(int(ySz / SF) / nR) * nR

        new[(x * 20) + 4] = file[(x * 20) + 4] % 256
        new[(x * 20) + 5] = file[(x * 20) + 5] % 256
        new[(x * 20) + 6] = file[(x * 20) + 6] % 256
        new[(x * 20) + 7] = file[(x * 20) + 7] % 256

        if xOff == 0 and yOff == 0 and xSz == 0 and ySz == 0 and (id in known or (id in advanced and target >=320)):
            # skip weather header
            print("Skip weather header")
            continue
        if xSz == 0 and ySz == 0 and (id in known or (id in advanced and target >=320)):
            # skip deleted items
            print("Skip zero size")
            continue

        new[(x * 20) + 9] = pint(nXoff // 256)
        new[(x * 20) + 8] = pint(nXoff) % 256

        new[(x * 20) + 11] = pint(nYoff // 256)
        new[(x * 20) + 10] = pint(nYoff)

        new[(x * 20) + 13] = pint(nXsz // 256)
        new[(x * 20) + 12] = pint(nXsz)

        new[(x * 20) + 15] = pint(nYsz // 256)
        new[(x * 20) + 14] = pint(nYsz)


        if id in advanced and cmd and id != 0x1B:
            new[(x * 20) + 4] = 0x09
            new[(x * 20) + 5] = 0x01
            new[(x * 20) + 6] = 0x00
            new[(x * 20) + 7] = 0x00

            new[(x * 20) + 13] = 0x00
            new[(x * 20) + 12] = 0x00
            new[(x * 20) + 15] = 0x00
            new[(x * 20) + 14] = 0x00
            print("Skip ", "{:02X}".format(id), " in compatibility mode")

            continue

        

        if dat == 0 and ((id in known and id != 0x19) or (id in advanced and target >=320) or (x == 0 and id == 0x19)):
            # maybe color, restore original and skip
            new[(x * 20) + 4] = pint(file[(x * 20) + 4])
            new[(x * 20) + 5] = pint(file[(x * 20) + 5])
            new[(x * 20) + 6] = pint(file[(x * 20) + 6])
            new[(x * 20) + 7] = pint(file[(x * 20) + 7])

            # dont restore original size
            # new[(x * 20) + 13] = pint(file[(x * 20) + 13])
            # new[(x * 20) + 12] = pint(file[(x * 20) + 12])
            # new[(x * 20) + 15] = pint(file[(x * 20) + 15])
            # new[(x * 20) + 14] = pint(file[(x * 20) + 14])

            new[(x * 20) + 16] = pint(file[(x * 20) + 16])
            new[(x * 20) + 17] = pint(file[(x * 20) + 17])
            new[(x * 20) + 18] = pint(file[(x * 20) + 18])
            new[(x * 20) + 19] = pint(file[(x * 20) + 19])

            new[(x * 20) + 20] = pint(file[(x * 20) + 20])
            new[(x * 20) + 21] = pint(file[(x * 20) + 21])
            new[(x * 20) + 22] = pint(file[(x * 20) + 22])
            new[(x * 20) + 23] = pint(file[(x * 20) + 23])
            print("Skip dat == 0 and known or background color")
            continue
        
        if xOff >= original:
            # cleanup if pushed out of screen
            new[(x * 20) + 4] = 0x09
            new[(x * 20) + 5] = 0x01
            new[(x * 20) + 6] = 0x00
            new[(x * 20) + 7] = 0x00

            new[(x * 20) + 13] = 0x00
            new[(x * 20) + 12] = 0x00
            new[(x * 20) + 15] = 0x00
            new[(x * 20) + 14] = 0x00
            print("Skip out of screen")
            continue


        try:
            if id in known or (id in advanced and target >= 320):
                sts = ""
                if id == 0x1B and (cmd or not (id in advanced and target >=320)):
                    print("Converting seconds to static icons")
                    sts = "0"
                    new[(x * 20) + 4] = 0x09
                    new[(x * 20) + 5] = 0x01
                    new[(x * 20) + 6] = 0x00
                    new[(x * 20) + 7] = 0x00

                    ySz = int(ySz / nR)
                    nYsz = int(nYsz / nR)

                    new[(x * 20) + 15] = pint(nYsz // 256)
                    new[(x * 20) + 14] = pint(nYsz)

                if id == 0x19:#force to image
                    new[(x * 20) + 4] = 0x09
                    cSz = 255
                    for z in range(xSz * ySz):
                        output.append(pint(file[(x * 20) + 16]))
                        output.append(pint(file[(x * 20) + 17]))
                elif dat == clt:
                    cSz = 255
                    for z in range(xSz * ySz):
                        output.append(pint(file[(z * 2) + dat]))
                        output.append(pint(file[(z * 2) + dat + 1]))
                else:
                    for z in range(xSz * ySz):
                        output.append(pint(file[clt + (pint(file[z + dat]) * 2)]))
                        output.append(pint(file[clt + (pint(file[z + dat]) * 2) + 1]))

                if id == 0x19 and lcs:
                    new[(x * 20) + 4] = 0x09
                    dts = bytearray()
                    for z in range(8):
                        dts.append(pint(file[(x * 20) + 16 + z]))
                    color_table, indexes = create_text(xSz, ySz, location, nXsz, nYsz, dts)
                else:
                    color_table, indexes = create_image(xSz, ySz, output, cSz, nXsz, nYsz, x != 0)



                nSize = len(new)
                cLen = len(color_table)
                cDat = len(indexes)
                nDat = nSize + cLen

                rtl = str(clt) + "-" + sts + str(dat)
                add = False
                if rtl in rcs:
                    old = rcs[rtl].split("-")
                    
                    nSize = int(old[0])
                    nDat = int(old[1])
                    
                else:
                    rcs[rtl] = str(nSize) + "-" + str(nDat)
                    add = True

                print("NW:", "{:02X}".format(new[(x * 20) + 4])," ", "No:", "{:02X}".format(nR), nXoff, " ", nYoff, " ", nXsz, " ", nYsz, " ", nSize, " ", nDat)

                new[(x * 20) + 19] = (nSize // (256**3)) & 0xFF
                new[(x * 20) + 18] = (nSize // (256**2)) & 0xFF
                new[(x * 20) + 17] = (nSize // 256) & 0xFF
                new[(x * 20) + 16] = (nSize) & 0xFF

                new[(x * 20) + 23] = (nDat // (256**3)) & 0xFF
                new[(x * 20) + 22] = (nDat // (256**2)) & 0xFF
                new[(x * 20) + 21] = (nDat // 256) & 0xFF
                new[(x * 20) + 20] = (nDat) & 0xFF

                #if exists no extending
                if add:
                    new.extend(color_table)
                    new.extend(indexes)
            else:
                new[(x * 20) + 4] = 0x09
                new[(x * 20) + 5] = 0x01
                new[(x * 20) + 6] = 0x00
                new[(x * 20) + 7] = 0x00

                new[(x * 20) + 13] = 0x00
                new[(x * 20) + 12] = 0x00
                new[(x * 20) + 15] = 0x00
                new[(x * 20) + 14] = 0x00

                new[(x * 20) + 19] = 0x00
                new[(x * 20) + 18] = 0x00
                new[(x * 20) + 17] = 0x00
                new[(x * 20) + 16] = 0x00

                new[(x * 20) + 23] = 0x00
                new[(x * 20) + 22] = 0x00
                new[(x * 20) + 21] = 0x00
                new[(x * 20) + 20] = 0x00

        except Exception as error:
            print(error)
            new[(x * 20) + 4] = 0x09
            new[(x * 20) + 5] = 0x01
            new[(x * 20) + 6] = 0x00
            new[(x * 20) + 7] = 0x00

            new[(x * 20) + 13] = 0x00
            new[(x * 20) + 12] = 0x00
            new[(x * 20) + 15] = 0x00
            new[(x * 20) + 14] = 0x00

            new[(x * 20) + 19] = 0x00
            new[(x * 20) + 18] = 0x00
            new[(x * 20) + 17] = 0x00
            new[(x * 20) + 16] = 0x00

            new[(x * 20) + 23] = 0x00
            new[(x * 20) + 22] = 0x00
            new[(x * 20) + 21] = 0x00
            new[(x * 20) + 20] = 0x00
        
    return new

def quantize(bitmap_bytes, num_colors):
    # Convert the byte array to a PIL Image
    image = Image.open(io.BytesIO(bitmap_bytes))

    # Resize and quantize the image using PIL
    resized_image = image.quantize(colors=num_colors)

    # Save the processed image as bytes
    result_byte_stream = io.BytesIO()
    resized_image.save(result_byte_stream, format="PNG")

    # Return the result as a byte array
    return result_byte_stream.getvalue()

def imageBytes(bitmap_bytes, num_colors):
    # convert
    image = Image.open(io.BytesIO(bitmap_bytes))
    transparent = True

    indexed_image = image.quantize(colors=num_colors).convert("P", palette=Image.ADAPTIVE) # create an indexed image , palette=Image.ADAPTIVE
    # # Retrieve the color table (palette)
    color_table = indexed_image.getpalette()

    rgb565_color_table = bytearray()

    for i in range(0, len(color_table), 3):
        # Extract the RGB components from RGB24
        r = color_table[i] >> 3
        g = color_table[i + 1] >> 2
        b = color_table[i + 2] >> 3

        # Calculate luminance
        luminance = 0.2126 * color_table[i] + 0.7152 * color_table[i + 1] + 0.0722 * color_table[i + 2]

        # Normalize luminance to a range of 0 to 100
        brightness_percentage = (luminance / 255) * 100

        # Check if brightness is above 50%
        if brightness_percentage < 0 and transparent:
            r = 0
            g = 0
            b = 0

        # Combine the components into RGB565 format (16 bits)
        rgb565_color = ((r << 11) | (g << 5) | b).to_bytes(2, byteorder='big')

        # Append the RGB565 color to the new color table
        rgb565_color_table.append(pint(rgb565_color[1]))
        rgb565_color_table.append(pint(rgb565_color[0]))

    rgb565_color_table = pad_byte_array(rgb565_color_table, num_colors * 2, 0x00)
    # print("color size swap: ", len(rgb565_color_table))
    # # Retrieve the indexes as a list
    indexes = list(indexed_image.getdata())

    data = bytearray()
    data.append(pint(len(rgb565_color_table)/255))
    data.append(pint(len(rgb565_color_table)%255))

    data.append(pint(len(indexes)/255))
    data.append(pint(len(indexes)%255))

    data.extend(rgb565_color_table)
    data.extend(indexes)


    return data



def get_bytes_from_file(filename):
    # print("Reading from: ", filename)
    return open(filename, "rb").read()


if __name__ == "__main__":
    print("Running test")
    if (len(sys.argv) > 1):
        name = sys.argv[1]
        loc = ""
        if len(sys.argv) > 2:
            loc = sys.argv[2]
        print("Start resizing watchface: ", name)
        if path.exists(sys.argv[1]):
            file = get_bytes_from_file(sys.argv[1])
            # print(file[0])
            # print(f"file size {len(file)}")


            new = resize(file, 240, 410, loc, False)

            with open(name.replace(".bin", "") + "-410.bin", 'wb') as f:
               f.write(new)
            print("Done, resized file name: ", name.replace(".bin", "") + "-resized.bin")
            print("#####################################################################")
        
        else:
            print("File not found: ", sys.argv[1])
    else:
        print("Specify the watchface bin file")
        print(">python test.py \"watchface.bin\"")