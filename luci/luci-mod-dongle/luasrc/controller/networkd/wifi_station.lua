module("luci.controller.networkd.wifi_station", package.seeall)

function index()
    entry({"networkd", "wifi_station"}, firstchild(), "WifiStation", 30).dependent=false
    entry({"networkd", "wifi_station", "welcome"}, call("welcome"), _("welcome"), 1)
    entry({"networkd", "wifi_station", "select"}, call("select"), _("select"), 2)
    entry({"networkd", "wifi_station", "wifi_list"}, call("wifi_list"), _("wifi_list"), 3)
    entry({"networkd", "wifi_station", "wifi_input"}, call("wifi_input"), nil, nil)
    entry({"networkd", "wifi_station", "connect"}, post("connect_ap"), nil, nil)
end


function welcome()
    luci.template.render("networkd/welcome")
end

function select()
    luci.template.render("networkd/select")
end

function wifi_list()
    local scanlist = luci.sys.exec("networkd_client -l")
    luci.template.render("networkd/wifi_list", {scanlist=scanlist})
end

function wifi_input()
    luci.template.render("networkd/wifi_input")
end

function connect_ap()
    local ssid = luci.http.formvalue("ssid")
    local password = luci.http.formvalue("password")
    luci.sys.exec(string.format("networkd_client -c -s \"%s\" -p \"%s\"", ssid, password))
    luci.http.redirect(luci.dispatcher.build_url("networkd", "wifi_station", "welcome"))
end

