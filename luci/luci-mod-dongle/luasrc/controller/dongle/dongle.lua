-- Copyright 2020 xujinfeng <xujinfeng@allwinnertech.com>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.dongle.dongle", package.seeall)

function index()
	local root = node()
	if not root.target then
		root.target = alias("dongle")
		root.index = true
	end

	page          = node()
	page.lock     = true
	page.target   = alias("dongle")
	page.subindex = true
	page.index    = false

	page          = node("dongle")
	page.title    = _("dongle")
	page.target   = alias("networkd", "wifi_station", "welcome")
	page.order    = 5
	page.setuser  = "root"
	page.setgroup = "root"
	page.index    = true

	entry({"dongle", "dongle"}, alias("networkd", "wifi_station", "welcome"), _("Connect Ap"), 20).index = true
end
