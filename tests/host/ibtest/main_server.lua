-- Iceball byte-order check, run by the dedicated server (tests/host/test_all.sh).
-- It loads the game's own maps, models and pictures and packs numbers, and
-- prints what it read; the PC and the 68k (big-endian) builds must print the
-- same. GPL-3.0-or-later, like Iceball. Copyright (c) 2026 AmiCradle Team.

local function h(acc, v)
	-- a small running hash that stays inside a double's exact integers
	return (acc * 31 + (v % 4294967296)) % 2147483647
end

local function out(...)
	print("IBTEST", ...)
end

-- maps: .vxl and .icemap, every pillar
local function map_sum(map)
	common.map_set(map)
	local xl, yl, zl = common.map_get_dims()
	local acc = 0
	for z = 0, zl - 1, 3 do
		for x = 0, xl - 1, 3 do
			local t = common.map_pillar_get(x, z)
			for i = 1, #t do acc = h(acc, t[i]) end
		end
	end
	return xl, yl, zl, acc
end

for _, name in ipairs({ "pkg/maps/mesa.vxl" }) do
	do
		local map = common.map_load(name, "vxl")
		out("map", name, map_sum(map))
		-- and round trip through IceMap
		common.map_save(map, "svsave/vol/ibtest.icemap", "icemap")
		local m2 = common.map_load("svsave/vol/ibtest.icemap", "icemap")
		out("icemap", name, map_sum(m2))
		local f = common.bin_load("svsave/vol/ibtest.icemap")
		local acc = 0
		for i = 1, #f, 7 do acc = h(acc, f:byte(i)) end
		out("icemap-bytes", #f, acc)
	end
end

-- models: every .pmf the game ships
local pmfs = { MODELS }
for _, name in ipairs(pmfs) do
	local pmf = common.model_load_pmf(name)
	local acc = 0
	for b = 0, common.model_len(pmf) - 1 do
		local bname, pts = common.model_bone_get(pmf, b)
		for i = 1, #pts do
			local p = pts[i]
			acc = h(acc, p.radius); acc = h(acc, p.x); acc = h(acc, p.y); acc = h(acc, p.z)
			acc = h(acc, p.r); acc = h(acc, p.g); acc = h(acc, p.b)
		end
		acc = h(acc, #pts)
	end
	out("pmf", name, common.model_len(pmf), acc)
	common.model_save_pmf(pmf, "svsave/vol/ibtest.pmf")
	local f = common.bin_load("svsave/vol/ibtest.pmf")
	local fa = 0
	for i = 1, #f do fa = h(fa, f:byte(i)) end
	out("pmf-save", name, #f, fa)
end

-- pictures: every .tga and .png
local imgs = { IMAGES }
for _, name in ipairs(imgs) do
	local fmt = name:sub(-3)
	local img = common.img_load(name, fmt)
	local w, hh = common.img_get_dims(img)
	local acc = 0
	for y = 0, hh - 1 do
		for x = 0, w - 1 do
			acc = h(acc, common.img_pixel_get(img, x, y))
		end
	end
	out("img", name, w, hh, acc)
end

-- packed numbers, as the game sends them
local function hex(s)
	return (s:gsub(".", function(c) return string.format("%02X", c:byte()) end))
end
local s = common.net_pack("bBhHiIfdz", -5, 250, -1234, 54321, -123456789, 3000000000, 1.5, -2.25, "hi")
out("pack", hex(s))
out("unpack", common.net_unpack("bBhHiIfdz", s))
local a = common.net_pack_array("h", 4, { 1, -2, 300, -32768 })
out("pack-array", hex(a))
local t = common.net_unpack_array("h", 4, a)
out("unpack-array", t[1], t[2], t[3], t[4])
local d = common.net_pack_array("d", 2, { 0.1, -1e10 })
out("pack-array-d", hex(d))
local td = common.net_unpack_array("d", 2, d)
out("unpack-array-d", td[1], td[2])

out("done")
-- the server keeps running: test_all.sh stops it at "done"
