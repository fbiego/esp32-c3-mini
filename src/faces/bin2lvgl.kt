/*
   MIT License

  Copyright (c) 2024 Felix Biego

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

  ______________  _____
  ___  __/___  /_ ___(_)_____ _______ _______
  __  /_  __  __ \__  / _  _ \__  __ `/_  __ \
  _  __/  _  /_/ /_  /  /  __/_  /_/ / / /_/ /
  /_/     /_.___/ /_/   \___/ _\__, /  \____/
                              /____/

*/

import java.awt.AlphaComposite
import java.awt.Color
import java.awt.Graphics2D
import java.awt.RenderingHints
import java.awt.image.BufferedImage
import java.awt.image.DataBufferInt
import java.io.File
import java.util.Calendar
import java.util.Random
import javax.imageio.ImageIO

fun byteArrayOfInts(vararg ints: Int) = ByteArray(ints.size) { pos -> ints[pos].toByte() }

fun Byte.toPInt() = toInt() and 0xFF

fun hex(b: Byte): String {
    return b.toPInt().toString(16).padStart(2, '0')
}

fun hex(i: Int): String {
    return i.toString(16).padStart(2, '0')
}

class Item(
        // var id: Int,
        var known: Boolean,
        var name: String
)

class Resource(var id: Int, var pos: Int)

class Show(var id: Int, var value: Int) {
    private var offset = 0
    fun getOff(max: Int): Int {
        var dig = getPlaceValue(value, offset)
        if (dig >= max) {
            dig = 0
        }
        offset++
        return dig
    }

    fun getPlaceValue(number: Int, position: Int): Int {
        val numberAsString = number.toString()
        // Check if the position is valid
        if (position < 0 || position >= numberAsString.length) {
            return 0
        }
        // Get the character at the specified position and convert it to an integer
        val digitChar = numberAsString[numberAsString.length - 1 - position]
        return digitChar.toString().toInt()
    }

    fun getLv(max: Int): String {
        val x = (Math.pow(10.0, (offset - 1.0))).toInt()
        return when (id) {
            0x00 -> "(hour / $x) % $max"
            0x01 -> "(minute / $x) % $max"
            0x02 -> "(day / $x) % $max"
            0x03 -> {
                val s = if (max == 12) {
                    "(month - 1)"
                } else {
                    "month"
                }
                "($s / $x) % $max"
            }
            0x06 -> "((weekday + 6) / $x) % $max"
            0x07 -> "(year / $x) % $max"
            0x08 -> "(am ? 0 : 1) % $max"
            0x0B -> "(battery / $x) % $max"
            0x1B -> "(seconds / $x) % $max"
            0x16 -> "(temp / $x) % $max"
            0x17 -> "icon % 8"
            0x10 -> "(bpm / $x) % $max"
            0x11 -> "(oxygen / $x) % $max"
            0x0E -> "(steps / $x) % $max"
            0x0F -> "(kcal / $x) % $max"
            0x14 -> "(distance / $x) % $max"
            else -> ""
        }
    }
}

fun group(id: Int): Int {
    return when (id) {
        0x00, 0x01, 0x02, 0x03, 0x06, 0x07, 0x08 -> 1 // time
        0x0A, 0x0B -> 2 // status
        0x0E, 0x0F, 0x14, 0x15 -> 3 // activity
        0x10, 0x11 -> 4 // health
        0x26, 0x16, 0x17, 0xFA -> 5 // weather
        else -> -1
    }
}

fun type(id: Int): Item {
    return when (id) {
        0x00 -> Item(true, "Hour no")
        0x01 -> Item(true, "Minute no")
        0x02 -> Item(true, "Date no")
        0x03 -> Item(true, "Month no")
        0x06 -> Item(true, "Weekday label")
        0x07 -> Item(true, "Year no -")
        0x08 -> Item(true, "AM/PM label")
        0x09 -> Item(true, "Image/Icon")
        0x0A -> Item(true, "Connection label")
        0x0B -> Item(true, "Battery no")
        0x0C -> Item(true, "Sleep label")
        0x0D -> Item(true, "Analog hands")
        0x0E -> Item(true, "Steps no")
        0x0F -> Item(true, "Calories no")
        0x10 -> Item(true, "Heart Rate no")
        0x11 -> Item(true, "SP02 no")
        0x13 -> Item(true, "Sleep no")
        0x14 -> Item(true, "Distance no")
        0x15 -> Item(true, "Distance Label")
        0x16 -> Item(true, "Weather no")
        0x17 -> Item(true, "Weather label")
        0x19 -> Item(true, "Solid color")
        0x1A -> Item(false, "Analog x5")
        0x1B -> Item(true, "Seconds")
        0x1D -> Item(true, "Unknown")
        0x1E -> Item(false, "Analog x8")
        0xFA -> Item(true, "Weather no+ label")
        0xFB -> Item(false, "Click")
        0xFD -> Item(true, "Animation")
        else -> Item(false, "0x" + hex(id) + " -> \"Unknown\"")
    }
}

private val itemShow = ArrayList<Show>()

private fun generateList() {
    val cal = Calendar.getInstance()
    itemShow.clear()
    itemShow.add(Show(0x00, cal.get(Calendar.HOUR_OF_DAY))) // hour
    itemShow.add(Show(0x01, cal.get(Calendar.MINUTE))) // minute
    itemShow.add(Show(0x1B, cal.get(Calendar.SECOND))) // second
    itemShow.add(Show(0x02, cal.get(Calendar.DAY_OF_MONTH))) // date
    itemShow.add(Show(0x03, cal.get(Calendar.MONTH) + 1)) // month
    val day = cal.get(Calendar.DAY_OF_WEEK) - 1
    itemShow.add(Show(0x06, if (day == 0) 6 else day - 1)) // weekday sun - mon -> mon - sun
    itemShow.add(Show(0x07, cal.get(Calendar.YEAR))) // year

    itemShow.add(Show(0x10, 72)) // bpm
    itemShow.add(Show(0x11, 98)) // sp02
    itemShow.add(Show(0x0E, 2735)) // steps
    itemShow.add(Show(0x0F, 163)) // kcal
    itemShow.add(Show(0x0B, 85)) // battery
    itemShow.add(Show(0x14, 157)) // distance
    itemShow.add(Show(0x16, 2222)) // weather temp

    itemShow.add(Show(0x13, 3008)) // sleep
}

fun main(args: Array<String>) {

    if (args.size > 0) {
        val data = File(args[0]).readBytes()

        if (data.size > 0) {
            val nm = args[0].replace(".bin", "").replace("-", "_").replace("_dial", "")

            val faceName = if (args.size > 1 ) {
                args[1]
            } else {
                nm.replace("_", " ")
            }

            val binary = if (args.size > 2) {
                args[2].toBooleanStrictOrNull() ?: false
            } else {
                false
            }
            extractComponents(data, nm, faceName, binary)

            println("-----Done-------")
        } else {
            println("Could not read from ${args[0]}")
        }
    } else {
        println("Specify the file name")
    }
}

fun extractComponents(data: ByteArray, name: String, faceName: String, binary: Boolean, wd: Int = 240, ht: Int = 240,) {
    val no =
            (data[3].toPInt() * 256 * 256 * 256) +
                    (data[2].toPInt() * 256 * 256) +
                    (data[1].toPInt() * 256) +
                    data[0].toPInt()

    
    println("Detected $no components")

    if (no > 100) {
        println("Watchface not valid, exiting")
        return
    }

    var elements = ""
    
    generateList()

    val canvas = BufferedImage(wd, ht, BufferedImage.TYPE_INT_ARGB)

    val graphics = canvas.createGraphics()
    graphics.color = Color.BLACK
    graphics.fillRect(0, 0, wd, ht)

    val rsc = arrayListOf<Resource>()

    var lan = 0

    var text = "Components List\n"
    var a = 0 // item
    var b = 0 //

    var wt = 0

    var tp = 0

    var use_raw = binary
    if (use_raw){
        println("Watchface images will be exported as .bin files, you need to upload them manually")
    } else {
        println("Watchface images will be included in the code as .c files")
    }
    var rPrefix = "S:" // lvgl drive letter path for image bin files

    var extern = ""
    var objects = ""
    var declare = ""
    var faceItems = ""
    var rscArray = ""
    var rscPathArray = ""
    var lvUpdateTime = ""
    var lvUpdateWeather = ""
    var lvUpdateStatus = ""
    var lvUpdateActivity = ""
    var lvUpdateHealth = ""
    var secondsType = "NULL"

    var weatherIc = "const lv_img_dsc_t *face_${name}_dial_img_weather[] = {\n"
    var weatherPathIc = "const char *face_${name}_dial_img_weather[] = {\n"
    var connIC = "const lv_img_dsc_t *face_${name}_dial_img_connection[] = {\n"
    var connPathIC = "const char *face_${name}_dial_img_connection[] = {\n"
    var weatherJson = "["
    var connJson = "["

    // loop through the components
    for (x in 0 until no) {

        val hexId =
                hex(data[(x * 20) + 4]) +
                        hex(data[(x * 20) + 5]) +
                        hex(data[(x * 20) + 6]) +
                        hex(data[(x * 20) + 7])

        val id = data[(x * 20) + 4].toPInt()
        var xOff = (data[(x * 20) + 9].toPInt() * 256) + data[(x * 20) + 8].toPInt()
        var yOff = (data[(x * 20) + 11].toPInt() * 256) + data[(x * 20) + 10].toPInt()
        val xSz = (data[(x * 20) + 13].toPInt() * 256) + data[(x * 20) + 12].toPInt()
        val ySz = (data[(x * 20) + 15].toPInt() * 256) + data[(x * 20) + 14].toPInt()

        val clt =
                (data[(x * 20) + 19].toPInt() * 256 * 256 * 256) +
                        (data[(x * 20) + 18].toPInt() * 256 * 256) +
                        (data[(x * 20) + 17].toPInt() * 256) +
                        data[(x * 20) + 16].toPInt()

        val dat =
                (data[(x * 20) + 23].toPInt() * 256 * 256 * 256) +
                        (data[(x * 20) + 22].toPInt() * 256 * 256) +
                        (data[(x * 20) + 21].toPInt() * 256) +
                        data[(x * 20) + 20].toPInt()

        val id2 = data[(x * 20) + 5].toPInt()

        var isG =
                (data[(x * 20) + 5].toPInt() and 0x80) ==
                        0x80 // check if is grouped in single resource
        if (id == 0x08){
            isG = true
        }
        val cmp = if (isG) data[(x * 20) + 5].toPInt() and 0x7F else 1
        // println("Group $cmp")

        val aOff = data[(x * 20) + 6].toPInt()

        val isM = (data[(x * 20) + 7].toPInt() and 0x80) == 0x80
        val cG = if (isM) data[(x * 20) + 7].toPInt() and 0x7F else 0
        if (isM) {
            lan++
        }

        if (tp == 0x09 && id == 0x09) {
            a++
        } else if (tp != id) {
            tp = id
            a++
        } else if (lan == 1) {
            a++
        }

        text += "$x\t$a\t$lan\t$hexId\t$xOff\t$yOff\t$xSz\t$ySz\t$clt\t$dat\t${type(id).name}\n"

        if (!type(id).known) {
            println("Skipping: $hexId ${type(id).name}")
            continue
        }

        var output = byteArrayOfInts()

        try {

            for (z in 0 until (xSz * ySz)) {
                if (id == 0x19 || (id == 0x16 && (id2 == 0x00 || id2 == 0x06))) {
                    output += data[(x * 20) + 16]
                    output += data[(x * 20) + 17]
                } else if (clt == dat) {
                    output += data[(z * 2) + dat]
                    output += data[(z * 2) + dat + 1]
                } else {
                    output += data[clt + (data[z + dat].toPInt() * 2)]
                    output += data[clt + (data[z + dat].toPInt() * 2) + 1]
                }
            }
        } catch (error: Exception) {
            println("error at $x -> $error")
            continue
        }

        if (xSz > 0 && ySz > 0) {

            var rs = rsc.singleOrNull { it.id == clt }

            var rscJson = ""


            var z =
                    if (rs != null) {
                        rs.pos
                    } else {
                        println("rs is null at clt $clt, using x $x")
                        x
                    }

            val drawable =
                    if (id == 0x0d) {
                        (lan == 1 || lan == 17 || lan == 33) // allow only specific analog hands
                    } else {
                        true // otherwise all items are drawable
                    }

            if (rs == null && drawable) {
                rsc.add(Resource(clt, x))

                var rscArr = "const lv_img_dsc_t *face_${name}_dial_img_${x}_${clt}_group[] = {\n"
                var rscPathArr = "const char *face_${name}_dial_img_${x}_${clt}_group[] = {\n"

                rscJson = "["

                // save assets & declare
                for (aa in 0 until cmp) {
                    declare += "\tLV_IMG_DECLARE(face_${name}_dial_img_${x}_${clt}_${aa});\n"
                    rscArr += "\t&face_${name}_dial_img_${x}_${clt}_${aa},\n"

                    rscPathArr += "\t\"${rPrefix}${name}_${x}_${clt}_${aa}.bin\",\n"
                    rscJson +=  "\"${rPrefix}${name}_${x}_${clt}_${aa}.bin\", "
                }
                rscArr += "};\n"
                rscPathArr += "};\n"
                rscJson = rscJson.dropLast(2)
                rscJson += "]"

                if (id == 0x17) {
                    weatherIc += "\t&face_${name}_dial_img_${x}_${clt}_0,\n"
                    weatherPathIc += "\t\"${rPrefix}${name}_${x}_${clt}_0.bin\",\n"
                    weatherJson += "\"${rPrefix}${name}_${x}_${clt}_0.bin\","
                } else if (id == 0x0A) {
                    connIC += "\t&face_${name}_dial_img_${x}_${clt}_0,\n"
                    connPathIC += "\t\"${rPrefix}${name}_${x}_${clt}_0.bin\",\n"
                    connJson += "\"${rPrefix}${name}_${x}_${clt}_0.bin\","

                    if (connIC.count { it == '\n' } > 2) {

                        rscArray += connIC + "};\n"
                        rscPathArray += connPathIC + "};\n"
                        rscJson = connJson.dropLast(1) + "]"
                    }
                } else if (cmp == 1) {
                    // do not add non grouped items
                } else {
                    // do not add weather to group
                    rscArray += rscArr
                    rscPathArray += rscPathArr
                }

                saveAsset(
                        output,
                        xSz,
                        ySz / cmp,
                        !(x == 0 && (id == 0x09 || id == 0x19)),
                        cmp,
                        name,
                        "${x}_${clt}",
                        use_raw
                )

                if (id == 0x1E) {} else {

                    // saveImage(output, xSz, ySz, x, name, clt)
                }
            } else if (id == 0x16 && id2 == 0x00) {
                

                saveAsset(
                        output,
                        xSz,
                        ySz / cmp,
                        !(x == 0 && (id == 0x09 || id == 0x19)),
                        cmp,
                        name,
                        "${x}_${clt}",
                        use_raw
                )
                rscJson = "["
                for (aa in 0 until cmp) {
                    rscJson +=  "\"${rPrefix}${name}_${z}_${clt}_${aa}.bin\", "
                }
                rscJson = rscJson.dropLast(2)
                rscJson += "]"
            } else {
                rscJson = "["
                for (aa in 0 until cmp) {
                    rscJson +=  "\"${rPrefix}${name}_${z}_${clt}_${aa}.bin\", "
                }
                rscJson = rscJson.dropLast(2)
                rscJson += "]"
            }

            if (cmp <= 1){
                rscJson = "null"
            }

            if (id == 0x0A) {
                if (connIC.count { it == '\n' } < 3) {
                    continue
                }
            }

            if (isM) {
                if (lan == cG) {
                    lan = 0
                } else if (id == 0x0d &&
                                (lan == 1 || lan == 32 || lan == 40 || lan == 17 || lan == 33)
                ) {
                    yOff -= (ySz - aOff)
                    xOff -= aOff
                } else {
                    continue
                }
            }

            if (id == 0x17) {
                wt++
                if (wt == 9) {
                    rscArray += weatherIc + "};\n"
                    rscPathArray += weatherPathIc + "};\n"

                    rscJson = weatherJson.dropLast(1) + "]"
                }
                if (wt != 1) {
                    continue
                }
            }

            if (id == 0x16 && id2 == 0x06) {
                continue
            }

            if (drawable) {
                // extern lv_obj_t *face_{{name}};
                extern += "\textern lv_obj_t *face_${name}_${x}_${clt};\n"
                objects += "lv_obj_t *face_${name}_${x}_${clt};\n"

                elements += jsonElem.replace("{{id}}", "$id")
                                    .replace("{{sub}}", "$id")
                                    .replace("{{x}}", "$xOff")
                                    .replace("{{y}}", "$yOff")
                                    .replace("{{pvX}}", "$aOff")
                                    .replace("{{pvY}}", "${ySz - aOff}")
                                    .replace("{{image}}", "${rPrefix}${name}_${z}_${clt}_0.bin")
                                    .replace("{{group}}", "$rscJson")

                faceItems +=
                        lvItem.replace("{{PARENT}}", "face_${name}")
                                .replace("{{CHILD}}", "face_${name}_${x}_${clt}")
                                .replace("{{CHILD_X}}", "$xOff")
                                .replace("{{CHILD_Y}}", "$yOff")
                                .replace("{{RESOURCE}}", if (use_raw) {"\"${rPrefix}${name}_${z}_${clt}_0.bin\""} else {"&face_${name}_dial_img_${z}_${clt}_0"})

                if (id == 0x0d) {
                    faceItems +=
                            "\tlv_img_set_pivot(face_${name}_${x}_${clt}, $aOff, ${ySz - aOff});\n"
                    if (lan == 1) {
                        // hour hand
                        lvUpdateTime +=
                                "\tlv_img_set_angle(face_${name}_${x}_${clt}, hour * 300 + (minute * 5) + (second * (5 / 60)));\n"
                    }
                    if (lan == 17) {
                        // minute hand
                        lvUpdateTime +=
                                "\tlv_img_set_angle(face_${name}_${x}_${clt}, (minute * 60) + second);\n"
                    }
                    if (lan == 33) {
                        // second hand
                        // lvUpdateTime += "\tlv_img_set_angle(face_${name}_${x}_${clt}, second * 60);\n"
                        secondsType = "&face_${name}_${x}_${clt}"
                    }
                }
            }
            if (id == 0x16 && id2 == 0x00) {
                lvUpdateWeather +=
                        "\tif (temp >= 0)\n\t{\n\t\tlv_obj_add_flag(face_${name}_${x}_${clt}, LV_OBJ_FLAG_HIDDEN);\n\t} else {\n\t\tlv_obj_clear_flag(face_${name}_${x}_${clt}, LV_OBJ_FLAG_HIDDEN);\n\t}\n"
                continue
            }
            if (id == 0x16 && id2 == 0x01) {
                continue
            }

            val sh = itemShow.singleOrNull { it.id == id }
            val offs = sh?.getOff(cmp) ?: Random().nextInt(cmp)
            val lvT = sh?.getLv(cmp) ?: ""
            if (lvT.isNotEmpty() && !(id == 0x0b && aOff == 0)) {
                when (group(id)) {
                    1 -> {
                        lvUpdateTime +=
                                "\tlv_img_set_src(face_${name}_${x}_${clt}, face_${name}_dial_img_${z}_${clt}_group[${lvT}]);\n"
                    }
                    2 -> {
                        lvUpdateStatus +=
                                "\tlv_img_set_src(face_${name}_${x}_${clt}, face_${name}_dial_img_${z}_${clt}_group[${lvT}]);\n"
                        if (lvT == "(battery / 100) % 10") {
                            lvUpdateStatus +=
                                    "\tif (battery < 100)\n\t{\n\t\tlv_obj_add_flag(face_${name}_${x}_${clt}, LV_OBJ_FLAG_HIDDEN);\n\t} else {\n\t\tlv_obj_clear_flag(face_${name}_${x}_${clt}, LV_OBJ_FLAG_HIDDEN);\n\t}\n"

                                    // do not draw it on the preview
                                    continue
                        }
                    }
                    3 -> {
                        lvUpdateActivity +=
                                "\tlv_img_set_src(face_${name}_${x}_${clt}, face_${name}_dial_img_${z}_${clt}_group[${lvT}]);\n"
                    }
                    4 -> {
                        lvUpdateHealth +=
                                "\tlv_img_set_src(face_${name}_${x}_${clt}, face_${name}_dial_img_${z}_${clt}_group[${lvT}]);\n"
                    }
                    5 -> {
                        lvUpdateWeather +=
                                "\tlv_img_set_src(face_${name}_${x}_${clt}, face_${name}_dial_img_${z}_${clt}_group[${lvT}]);\n"
                    }
                }
            }
            if (id == 0x17) {
                lvUpdateWeather +=
                        "\tlv_img_set_src(face_${name}_${x}_${clt}, face_${name}_dial_img_weather[icon % 8]);\n"
            }
            if (id == 0x0b && aOff == 0) {
                lvUpdateStatus +=
                        "\tlv_img_set_src(face_${name}_${x}_${clt}, face_${name}_dial_img_${z}_${clt}_group[(battery / (100 / ${cmp})) % ${cmp}]);\n"
            }
            if (id == 0x0a) {
                lvUpdateStatus +=
                        "\tlv_img_set_src(face_${name}_${x}_${clt}, face_${name}_dial_img_connection[(connection ? 0 : 1) % 2]);\n"
            }
            if (id == 0x08) {
                lvUpdateTime +=
                        "\tif (mode)\n\t{\n\t\tlv_obj_add_flag(face_${name}_${x}_${clt}, LV_OBJ_FLAG_HIDDEN);\n\t} else {\n\t\tlv_obj_clear_flag(face_${name}_${x}_${clt}, LV_OBJ_FLAG_HIDDEN);\n\t}\n"
                lvUpdateTime +=
                        "\tlv_img_set_src(face_${name}_${x}_${clt}, face_${name}_dial_img_${z}_${clt}_group[(am ? 0 : 1) % 2]);\n"
            }

            if (id == 0x0d && (lan == 17 || lan == 33)) {
                continue // do not draw the analog() on the preview
            }
            if (xSz > 500 || ySz > 5000) {
                // not valid ?
            } else {
                val image = getImage(output, xSz, ySz / cmp, offset = offs)
                graphics.drawImage(image, xOff, yOff, null)
            }
        }
    }

    graphics.dispose()


    // Create a file to save the image to
    val dir = File(name)

    if (!dir.exists()) {
        dir.mkdirs()
        println("Created output folder")
    }

    val list = File(dir, "items.txt")
    list.writeText(text)

    if (use_raw){
        val jsonFile = File(dir, "${name.lowercase()}.json")
        elements = elements.dropLast(1)
        jsonObj = jsonObj.replace("{{name}}", name.lowercase()).replace("{{elements}}", elements)
        jsonFile.writeText(jsonObj)
    }
    

    val outputFile = File(dir, "watchface.png")

    // Save the BufferedImage to the output file
    ImageIO.write(canvas, "png", outputFile)

    val scaledCanvas = BufferedImage(160, 160, BufferedImage.TYPE_INT_ARGB)
    val g2d: Graphics2D = scaledCanvas.createGraphics()
    g2d.setRenderingHint(
            RenderingHints.KEY_INTERPOLATION,
            RenderingHints.VALUE_INTERPOLATION_BILINEAR
    )
    g2d.setRenderingHint(RenderingHints.KEY_RENDERING, RenderingHints.VALUE_RENDER_QUALITY)
    g2d.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON)
    g2d.drawImage(canvas, 0, 0, 160, 160, null)
    val mask = BufferedImage(160, 160, BufferedImage.TYPE_INT_ARGB)
    val maskGraphics = mask.createGraphics()
    maskGraphics.color = Color.BLACK
    maskGraphics.fillOval(0, 0, 160, 160)
    maskGraphics.dispose()
    val composite = AlphaComposite.DstIn
    g2d.composite = composite
    g2d.drawImage(mask, 0, 0, null)
    g2d.dispose()

    saveAsset(bufferBytes(scaledCanvas), 160, 160, false, 1, name, "preview", false)

    declare += "\tLV_IMG_DECLARE(face_${name}_dial_img_preview_0);\n"

    if (use_raw) {
        declare = "LV_IMG_DECLARE(face_${name}_dial_img_preview_0);\n"
    } else { 
        declare += "\tLV_IMG_DECLARE(face_${name}_dial_img_preview_0);\n"
    }

    h_file =
            h_file.replace("{{NAME}}", name.uppercase())
                    .replace("{{name}}", name.lowercase())
                    .replace("{{EXTERN}}", extern)
                    .replace("{{DECLARE}}", declare)
                    .replace("{{FACE_NAME}}", faceName)
    c_file =
            c_file.replace("{{NAME}}", name.uppercase())
                    .replace("{{name}}", name.lowercase())
                    .replace("{{OBJECTS}}", objects)
                    .replace("{{ITEMS}}", faceItems)
                    .replace("{{RSC_ARR}}", if (use_raw) { "" } else { rscArray })
                    .replace("{{RSC_PATH_ARR}}", if (use_raw) { rscPathArray } else { ""})
                    .replace("{{TIME}}", lvUpdateTime)
                    .replace("{{STATUS}}", lvUpdateStatus)
                    .replace("{{WEATHER}}", lvUpdateWeather)
                    .replace("{{ACTIVITY}}", lvUpdateActivity)
                    .replace("{{HEALTH}}", lvUpdateHealth)
                    .replace("{{FACE_NAME}}", faceName)
                    .replace("{{SECOND}}", secondsType)

    val header = File(dir, "$name.h")
    val source = File(dir, "$name.c")
    header.writeText(h_file)
    source.writeText(c_file)
}

fun getImage(
        rgb565: ByteArray,
        width: Int,
        height: Int,
        tr: Boolean = true,
        offset: Int = 0
): BufferedImage {
    // Convert the RGB565 byte array to a BufferedImage with alpha channel
    val image = BufferedImage(width, height, BufferedImage.TYPE_INT_ARGB)
    val raster = image.raster
    val dataBuffer = raster.dataBuffer as DataBufferInt
    val pixels = dataBuffer.data

    for (y in 0 until height) {
        for (x in 0 until width) {
            val i = y * width + x
            val j = y * width + x + (height * width * offset)
            val r = (rgb565[j * 2 + 1].toInt() and 0xF8)
            val g =
                    ((rgb565[j * 2 + 1].toInt() and 0x07) shl 5) or
                            ((rgb565[j * 2].toInt() and 0xE0) shr 3)
            val b = (rgb565[j * 2].toInt() and 0x1F) shl 3

            // Set the alpha channel to 0 for black color
            val alpha = if (r == 0 && g == 0 && b == 0 && tr) 0 else 255

            // Pack the ARGB values into a single int
            val argb = (alpha shl 24) or (r shl 16) or (g shl 8) or b

            pixels[i] = argb
        }
    }

    // return BufferedImage
    return image
}

fun saveImage(rgb565: ByteArray, width: Int, height: Int, no: Int, folder: String, id: Int) {
    // Convert the RGB565 byte array to a BufferedImage
    val image = BufferedImage(width, height, BufferedImage.TYPE_USHORT_565_RGB)
    val dataBuffer = image.raster.dataBuffer as java.awt.image.DataBufferUShort
    for (y in 0 until height) {
        for (x in 0 until width) {
            val i = y * width + x
            val px = (rgb565[i * 2 + 1].toPInt() * 256) + rgb565[i * 2].toPInt()
            dataBuffer.setElem(i, px)
        }
    }

    // Write the BufferedImage to a JPG file
    val dir = File(folder)

    if (!dir.exists()) {
        dir.mkdirs()
        println("Created output folder")
    }

    val outputFile = File(dir, "$no-$id.jpg")
    ImageIO.write(image, "jpg", outputFile)
}

fun bufferBytes(canvas: BufferedImage): ByteArray {
    val width = canvas.width
    val height = canvas.height
    val pixels = IntArray(width * height)
    canvas.getRGB(0, 0, width, height, pixels, 0, width)

    var byteArray = byteArrayOfInts()

    for (pixel in pixels) {
        val red = (pixel shr 19) and 0x1F // Extract red component (5 bits)
        val green = (pixel shr 10) and 0x3F // Extract green component (6 bits)
        val blue = (pixel shr 3) and 0x1F // Extract blue component (5 bits)
        val alpha = (pixel shr 24) and 0xFF // Extract alpha component (8 bits)

        // Convert to RGB565 format with alpha
        val rgb565WithAlpha = (alpha shl 16) or (red shl 11) or (green shl 5) or blue

        // Swap the bytes (Little Endian format)
        byteArray += (rgb565WithAlpha and 0xFF).toByte()
        byteArray += ((rgb565WithAlpha shr 8) and 0xFF).toByte()
    }

    return byteArray
}

fun lvHeaderBytes(cf: Int, w: Int, h: Int): ByteArray {
    require(cf in 0..31) { "cf must be between 0 and 31" }
    require(w in 0..2047) { "w must be between 0 and 2047" }
    require(h in 0..2047) { "h must be between 0 and 2047" }

    val alwaysZero = 0
    val reserved = 0

    // Pack the values into a single 32-bit integer
    val header = (cf and 0x1F) or
                 ((alwaysZero and 0x07) shl 5) or
                 ((reserved and 0x03) shl 8) or
                 ((w and 0x07FF) shl 10) or
                 ((h and 0x07FF) shl 21)

    // Convert the 32-bit integer to bytes in little-endian format
    return ByteArray(4) { i -> (header shr (i * 8) and 0xFF).toByte() }
}

fun saveAsset(
        rgb565: ByteArray,
        width: Int,
        height: Int,
        tr: Boolean = true,
        amount: Int,
        name: String,
        asset: String,
        binary: Boolean = false
) {

    val dir = File(name)

    if (!dir.exists()) {
        dir.mkdirs()
        println("Created output folder")
    }


    var text =
            asset_header.replace("{{NAME}}", name.uppercase()).replace("{{name}}", name.lowercase())
    

    for (a in 0 until amount) {
        var data_raw = byteArrayOfInts()

        var dat =
                """
// LVGL 9 format (RGB565) // LVGL_9 compatible 
const LV_ATTRIBUTE_MEM_ALIGN uint8_t face_${name}_dial_img_${asset}_data_${a}[] = {
{{BYTES}}
    };
        
"""
        var bts = "\t//RGB565 data \n\t"
        var btsA = "\n\n\t// Alpha Channel \n\t"
        var z = 1
        var zA = 1
        for (y in 0 until height) {
            for (x in 0 until width) {
                // val i = y * width + x
                val j = y * width + x + (height * width * a)
                val r = (rgb565[j * 2 + 1].toInt() and 0xF8)
                val g =
                        ((rgb565[j * 2 + 1].toInt() and 0x07) shl 5) or
                                ((rgb565[j * 2].toInt() and 0xE0) shr 3)
                val b = (rgb565[j * 2].toInt() and 0x1F) shl 3

                // Set the alpha channel to 0 for black color
                val alpha = if (r == 0 && g == 0 && b == 0 && tr) 0 else 255

                var hex =
                        String.format(
                                "0x%02X,0x%02X,",
                                rgb565[j * 2].toInt() and 0xFF,
                                rgb565[j * 2 + 1].toInt() and 0xFF
                        )
                data_raw += (rgb565[j * 2 + 1]).toByte()
                data_raw += (rgb565[j * 2]).toByte()
                if (tr) {
                    btsA += String.format("0x%02X,", alpha and 0xFF)
                    if (zA % 64 == 0 && zA != 0) {
                        btsA += "\n\t"
                    }
                    zA++
                    data_raw += (alpha).toByte()
                }
                if (z % 32 == 0 && z != 0) {
                    hex += "\n\t"
                }
                bts += hex
                z++
            }
        }

        bts = bts + btsA

        dat = dat.replace("{{BYTES}}", bts)

        text += dat + "\n"

        val (color, cf) =
                if (tr) {
                    Pair("LV_COLOR_FORMAT_NATIVE_WITH_ALPHA", 5)
                } else {
                    Pair("LV_COLOR_FORMAT_NATIVE", 4)
                }

        val obj =
                """
const lv_img_dsc_t face_${name}_dial_img_${asset}_${a} = {
    .header.magic = LV_IMAGE_HEADER_MAGIC,
    .header.w = $width,
    .header.h = $height,
    .data_size = sizeof(face_${name}_dial_img_${asset}_data_${a}),
    .header.cf = $color,
    .data = face_${name}_dial_img_${asset}_data_${a}};

    """

        text += obj

        if (binary){
            val hdr = lvHeaderBytes(cf, width, height)

            val dirB = File(dir, "binary")
            if (!dirB.exists()) {
                dirB.mkdirs()
                println("Created binary folder")
            }

            val rf = File(dirB, "${name}_${asset}_${a}.bin")
            rf.writeBytes(hdr + data_raw)
        }
    }

    if (!binary){
        val dirA = File(dir, "assets")
        if (!dirA.exists()) {
            dirA.mkdirs()
            println("Created assets folder")
        }

        val fl = File(dirA, "face_${name}_dial_img_${asset}.c")
        fl.writeText(text)
    }
}

var jsonElem =
"""
		{
			"id": {{id}},
			"x": {{x}},
			"y": {{y}},
            "pvX": {{pvX}},
			"pvY": {{pvY}},
			"image": "{{image}}",
			"group": {{group}}
		},"""

var jsonObj = 
"""
{
	"name": "{{name}}",
	"elements": [
		{{elements}}
	]
}
"""

var lvItem =
        """
    {{CHILD}} = lv_img_create({{PARENT}});
    lv_img_set_src({{CHILD}}, {{RESOURCE}});
    lv_obj_set_width({{CHILD}}, LV_SIZE_CONTENT);
    lv_obj_set_height({{CHILD}}, LV_SIZE_CONTENT);
    lv_obj_set_x({{CHILD}}, {{CHILD_X}});
    lv_obj_set_y({{CHILD}}, {{CHILD_Y}});
    lv_obj_add_flag({{CHILD}}, LV_OBJ_FLAG_ADV_HITTEST );
    lv_obj_clear_flag({{CHILD}}, LV_OBJ_FLAG_SCROLLABLE );
"""

var h_file =
        """
// File generated by bin2lvgl
// developed by fbiego. 
// https://github.com/fbiego
// Watchface: {{NAME}}

#ifndef _FACE_{{NAME}}_H
#define _FACE_{{NAME}}_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"

//#define ENABLE_FACE_{{NAME}} // ({{FACE_NAME}}) uncomment to enable or define it elsewhere

#ifdef ENABLE_FACE_{{NAME}}
    extern lv_obj_t *face_{{name}};
{{EXTERN}}

{{DECLARE}}

#endif
    void onFaceEvent(lv_event_t * e);

    void init_face_{{name}}(void (*callback)(const char*, const lv_img_dsc_t *, lv_obj_t **, lv_obj_t **));
    void update_time_{{name}}(int second, int minute, int hour, bool mode, bool am, int day, int month, int year, int weekday);
    void update_weather_{{name}}(int temp, int icon);
    void update_status_{{name}}(int battery, bool connection);
    void update_activity_{{name}}(int steps, int distance, int kcal);
    void update_health_{{name}}(int bpm, int oxygen);
    void update_all_{{name}}(int second, int minute, int hour, bool mode, bool am, int day, int month, int year, int weekday, 
                int temp, int icon, int battery, bool connection, int steps, int distance, int kcal, int bpm, int oxygen);
    void update_check_{{name}}(lv_obj_t *root, int second, int minute, int hour, bool mode, bool am, int day, int month, int year, int weekday, 
                int temp, int icon, int battery, bool connection, int steps, int distance, int kcal, int bpm, int oxygen);


#ifdef __cplusplus
}
#endif

#endif
"""

var c_file =
        """
// File generated by bin2lvgl
// developed by fbiego. 
// https://github.com/fbiego
// Watchface: {{NAME}}

#include "{{name}}.h"

#ifdef ENABLE_FACE_{{NAME}}

lv_obj_t *face_{{name}};
{{OBJECTS}}

#if LV_COLOR_DEPTH != 16
#error "LV_COLOR_DEPTH should be 16bit for watchfaces"
#endif

{{RSC_ARR}}

{{RSC_PATH_ARR}}

#endif

void init_face_{{name}}(void (*callback)(const char*, const lv_img_dsc_t *, lv_obj_t **, lv_obj_t **)){
#ifdef ENABLE_FACE_{{NAME}}
    face_{{name}} = lv_obj_create(NULL);
    lv_obj_clear_flag(face_{{name}}, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(face_{{name}}, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(face_{{name}}, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(face_{{name}}, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(face_{{name}}, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(face_{{name}}, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(face_{{name}}, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(face_{{name}}, 0, LV_PART_MAIN| LV_STATE_DEFAULT);

    lv_obj_add_event_cb(face_{{name}}, onFaceEvent, LV_EVENT_ALL, NULL);

    {{ITEMS}}

    callback("{{FACE_NAME}}", &face_{{name}}_dial_img_preview_0, &face_{{name}}, {{SECOND}});

#endif
}

void update_time_{{name}}(int second, int minute, int hour, bool mode, bool am, int day, int month, int year, int weekday)
{
#ifdef ENABLE_FACE_{{NAME}}
    if (!face_{{name}})
    {
        return;
    }
{{TIME}}
#endif
}

void update_weather_{{name}}(int temp, int icon)
{
#ifdef ENABLE_FACE_{{NAME}}
    if (!face_{{name}})
    {
        return;
    }
{{WEATHER}}
#endif
}

void update_status_{{name}}(int battery, bool connection){
#ifdef ENABLE_FACE_{{NAME}}
    if (!face_{{name}})
    {
        return;
    }
{{STATUS}}
#endif
}

void update_activity_{{name}}(int steps, int distance, int kcal)
{
#ifdef ENABLE_FACE_{{NAME}}
    if (!face_{{name}})
    {
        return;
    }
{{ACTIVITY}}
#endif
}

void update_health_{{name}}(int bpm, int oxygen)
{
#ifdef ENABLE_FACE_{{NAME}}
    if (!face_{{name}})
    {
        return;
    }
{{HEALTH}}
#endif
}

void update_all_{{name}}(int second, int minute, int hour, bool mode, bool am, int day, int month, int year, int weekday, 
    int temp, int icon, int battery, bool connection, int steps, int distance, int kcal, int bpm, int oxygen)
{
#ifdef ENABLE_FACE_{{NAME}}
    update_time_{{name}}(second, minute, hour, mode, am, day, month, year, weekday);
    update_weather_{{name}}(temp, icon);
    update_status_{{name}}(battery, connection);
    update_activity_{{name}}(steps, distance, kcal);
    update_health_{{name}}(bpm, oxygen);
#endif
}

void update_check_{{name}}(lv_obj_t *root, int second, int minute, int hour, bool mode, bool am, int day, int month, int year, int weekday, 
    int temp, int icon, int battery, bool connection, int steps, int distance, int kcal, int bpm, int oxygen)
{
#ifdef ENABLE_FACE_{{NAME}}
    if (root != face_{{name}})
    {
        return;
    }
    update_time_{{name}}(second, minute, hour, mode, am, day, month, year, weekday);
    update_weather_{{name}}(temp, icon);
    update_status_{{name}}(battery, connection);
    update_activity_{{name}}(steps, distance, kcal);
    update_health_{{name}}(bpm, oxygen);
#endif
}


"""

var asset_header =
        """
// File generated by bin2lvgl
// developed by fbiego. 
// https://github.com/fbiego
// Watchface: {{NAME}}

#include "../{{name}}.h"

#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif

"""
