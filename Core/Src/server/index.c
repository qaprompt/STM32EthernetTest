#include "server.h"
#include "mongoose.h"

static const char header_html[] = "content-type:text/html\r\ncontent-encoding:gzip\r\n";
static const char header_js[] = "content-type:text/javascript\r\ncontent-encoding:gzip\r\n";
static const char header_css[] = "content-type:text/css\r\ncontent-encoding:gzip\r\n";

// "<!DOCTYPE html><html><head><link rel=stylesheet href=main.css></head><body><h1>ERROR 404</h1> The requested resource was not found. </body></html>"
static const unsigned char FourOhFour_html[] = {0x1f, 0x8b, 0x08, 0x00, 0xbe, 0x70, 0x63, 0x62, 0x02, 0xff, 0x2d, 0x8d, 0xc1, 0x0a, 0x83, 0x30, 0x10, 0x44, 0x7f, 0x65, 0xfb, 0x03, 0xa6, 0x82, 0xc7, 0x35, 0x97, 0xd6, 0xb3, 0x25, 0x78, 0xe9, 0x31, 0x35, 0x2b, 0x91, 0xc6, 0x04, 0xb3, 0x1b, 0x8a, 0x7f, 0xdf, 0x50, 0x7b, 0x19, 0x06, 0x66, 0xde, 0x0c, 0x5e, 0xee, 0xe3, 0x6d, 0x7a, 0x3e, 0x06, 0xf0, 0xb2, 0x05, 0x8d, 0x7f, 0x25, 0xeb, 0x34, 0x86, 0x35, 0xbe, 0x21, 0x53, 0xe8, 0x59, 0x8e, 0x40, 0xec, 0x89, 0x04, 0x7c, 0xa6, 0xa5, 0xdf, 0xec, 0x1a, 0x9b, 0x99, 0x59, 0xa3, 0x3a, 0x9b, 0xaf, 0xe4, 0x8e, 0x4a, 0xb5, 0x7a, 0x30, 0x66, 0x34, 0xd0, 0x5d, 0xbb, 0x9a, 0xb4, 0x1a, 0x26, 0x4f, 0x75, 0x60, 0x2f, 0xc4, 0x42, 0xae, 0x3a, 0x4e, 0x25, 0xcf, 0x04, 0x1f, 0xcb, 0x10, 0x93, 0xc0, 0x92, 0x4a, 0x74, 0x0d, 0xa0, 0x3a, 0x79, 0xf5, 0x3b, 0xff, 0x02, 0x43, 0xba, 0x27, 0xe4, 0x92, 0x00, 0x00, 0x00};

// "<!DOCTYPE html><html><head><link rel=stylesheet href=main.css><meta name=viewport content=\"width=device-width, initial-scale=1\"></head><body><h1 id=deviceTitleText>Connecting...</h1><hr><div class=tab><button class=tablinks onclick=\"selectTab(event, 'Status')\" id=defaultOpen>Status</button><button class=tablinks onclick=\"selectTab(event, 'Settings')\">Settings</button></div><div id=Status class=tabcontent><table id=dataTable><tr><th style=width:100px>Data</th><th style=width:200px>Value</th></tr></table><hr></div><div id=Settings class=tabcontent><table><tr><td><label for=name>Device Name</label></td><td><input type=text id=deviceName name=deviceName required minlength=1 maxlength=15 size=10></td></tr><tr><td><label for=name>Device IP</label></td><td><input type=text id=ucIPAddress name=ucIPAddress minlength=7 maxlength=15 size=15></td></tr><tr><td><label for=name>Device Subnet Mask</label></td><td><input type=text id=ucNetMask name=ucNetMask minlength=7 maxlength=15 size=15></td></tr><tr><td><label for=name>Device Gateway IP</label></td><td><input type=text id=ucGatewayAddress name=ucGatewayAddress minlength=7 maxlength=15 size=15></td></tr><tr><td><label for=name>NT4 Server IP</label></td><td><input type=text id=nt4ServerAddress name=nt4ServerAddress minlength=7 maxlength=15 size=15></td></tr></table><hr><table><tr><td><button onclick=onSettingsReset()>Reset</button></td><td><button onclick=onSettingsSubmit()>Submit</button></td></tr></table></div><div id=myModal class=modal><div class=modal-content><span class=close>&times;</span><p><div id=modalText></div></p></div></div><script src=modal.js></script><script src=tabs.js></script><script src=settings.js></script><script src=websocketData.js></script><script src=main.js></script></body></html>"
static const unsigned char index_html[] = {0x1f, 0x8b, 0x08, 0x00, 0xbe, 0x70, 0x63, 0x62, 0x02, 0xff, 0xad, 0x95, 0x5b, 0x6f, 0xd3, 0x30, 0x14, 0x80, 0xff, 0x8a, 0xe9, 0x03, 0xdb, 0xa4, 0xb5, 0x69, 0xd1, 0x26, 0x24, 0x70, 0x2c, 0xa1, 0x15, 0xa1, 0x3d, 0xac, 0xab, 0x68, 0x85, 0xc4, 0xa3, 0xe3, 0x9c, 0x2e, 0xa6, 0x8e, 0x13, 0xe2, 0x93, 0x5e, 0xf8, 0xf5, 0xf8, 0x92, 0x2c, 0x4d, 0x47, 0x51, 0x41, 0x7b, 0x49, 0xec, 0x73, 0xfd, 0x7c, 0x39, 0xc7, 0xf4, 0xcd, 0xf4, 0xf1, 0x6e, 0xf9, 0x7d, 0xfe, 0x99, 0x64, 0x98, 0x2b, 0x46, 0x9b, 0x2f, 0xf0, 0x94, 0x51, 0x25, 0xf5, 0x9a, 0x54, 0xa0, 0x62, 0x83, 0x7b, 0x05, 0x26, 0x03, 0x40, 0x92, 0x55, 0xb0, 0x8a, 0x73, 0x2e, 0xf5, 0x48, 0x18, 0xc3, 0x68, 0x0e, 0xc8, 0x89, 0xe6, 0x39, 0xc4, 0x1b, 0x09, 0xdb, 0xb2, 0xa8, 0x90, 0x88, 0x42, 0x23, 0x68, 0x8c, 0x07, 0x5b, 0x99, 0x62, 0x16, 0xa7, 0xb0, 0x91, 0x02, 0x86, 0x7e, 0x72, 0x4d, 0xa4, 0x96, 0x28, 0xb9, 0x1a, 0x1a, 0xc1, 0x15, 0xc4, 0x93, 0x01, 0xa3, 0x51, 0xc8, 0x95, 0x14, 0xe9, 0xde, 0xe6, 0x9d, 0x10, 0x99, 0x36, 0x2e, 0x4b, 0x89, 0x0a, 0x96, 0xb0, 0x43, 0x76, 0x57, 0x68, 0x0d, 0x02, 0xa5, 0x7e, 0x1a, 0x8d, 0x46, 0xd6, 0x61, 0x62, 0x0d, 0x2b, 0x46, 0x53, 0xb9, 0x21, 0x42, 0x71, 0x63, 0x62, 0xe4, 0x89, 0x8d, 0x50, 0x23, 0x16, 0xba, 0x93, 0x38, 0x7a, 0x43, 0x0a, 0x2d, 0x94, 0x14, 0xeb, 0x78, 0x60, 0x40, 0xd9, 0x18, 0x4b, 0x9e, 0x5c, 0xc2, 0xc6, 0xe2, 0x5d, 0x93, 0x8b, 0x05, 0x72, 0xac, 0xcd, 0xc5, 0xd5, 0x20, 0xe4, 0x5c, 0xf1, 0x5a, 0xe1, 0x63, 0x09, 0x9a, 0x05, 0x05, 0x8d, 0x42, 0xc4, 0xff, 0x89, 0x0c, 0xe8, 0x68, 0x5d, 0x6c, 0xd6, 0x8e, 0xbb, 0x70, 0x91, 0x05, 0x0f, 0xf4, 0x36, 0x6f, 0xc8, 0xd5, 0xc5, 0x6e, 0x76, 0x8f, 0x51, 0x97, 0x07, 0x3c, 0x19, 0x47, 0xbe, 0x74, 0x13, 0x2b, 0xb3, 0xab, 0xc6, 0x8c, 0xf8, 0xe3, 0x88, 0xfd, 0x8e, 0x7e, 0x98, 0x8c, 0xc7, 0xe5, 0x8e, 0x4d, 0xad, 0x0d, 0x8d, 0x30, 0x7b, 0xa1, 0x7e, 0xe7, 0xd5, 0xdf, 0xb8, 0xaa, 0x21, 0xe8, 0x23, 0x17, 0x23, 0xc2, 0x10, 0xcf, 0xed, 0x62, 0x9f, 0xa6, 0x81, 0x3d, 0xc5, 0xd3, 0x20, 0xb8, 0xbb, 0xc1, 0x13, 0x50, 0x64, 0x55, 0x54, 0xb1, 0x3b, 0x7d, 0x36, 0xf5, 0x47, 0x46, 0x66, 0x76, 0x4c, 0x23, 0xaf, 0x73, 0x59, 0xd2, 0x60, 0x2b, 0x75, 0x59, 0x23, 0xc1, 0x7d, 0x09, 0x31, 0xda, 0xf3, 0xec, 0x8e, 0xd8, 0x99, 0x87, 0xdb, 0x73, 0x30, 0xaf, 0xe0, 0x67, 0x2d, 0x2b, 0x48, 0x49, 0x2e, 0xb5, 0x02, 0xfd, 0x64, 0xef, 0xd0, 0x84, 0xe4, 0x7c, 0xd7, 0x8e, 0x6f, 0x89, 0x91, 0xbf, 0xec, 0xdd, 0x19, 0x37, 0x19, 0xfc, 0x8a, 0xfe, 0x8e, 0x75, 0x3f, 0x3f, 0x0b, 0xaa, 0x16, 0xf7, 0xf3, 0x4f, 0x69, 0x5a, 0x81, 0x31, 0x81, 0xea, 0x50, 0xd0, 0xd1, 0xbc, 0xff, 0x13, 0xcd, 0xed, 0xf9, 0x34, 0x8b, 0x3a, 0xd1, 0xb6, 0x92, 0x1e, 0xb8, 0x59, 0x9f, 0x89, 0x35, 0x03, 0x74, 0xd6, 0x2d, 0x54, 0x3b, 0x7d, 0x3d, 0xa4, 0x2f, 0x1c, 0x61, 0xcb, 0xf7, 0xe7, 0x6f, 0x54, 0xe3, 0x70, 0xb4, 0x5b, 0x47, 0xd2, 0x57, 0xe0, 0x9b, 0x2d, 0x6f, 0xc8, 0x02, 0xaa, 0x0d, 0x54, 0xe7, 0xb2, 0x69, 0xbc, 0x09, 0x0e, 0x3d, 0xb6, 0x17, 0xd2, 0x7f, 0x61, 0x3b, 0x2c, 0x97, 0xa3, 0x32, 0x68, 0x5a, 0x43, 0xdb, 0x0b, 0x0a, 0xdd, 0x56, 0xd0, 0x57, 0x30, 0x80, 0x97, 0x57, 0xcc, 0xff, 0x0f, 0x6a, 0xbf, 0x25, 0x3f, 0xe9, 0x68, 0x6f, 0x47, 0x2e, 0x9d, 0x67, 0x18, 0x1c, 0xb9, 0xf6, 0x78, 0x7a, 0xa5, 0x9b, 0xef, 0x1f, 0x8a, 0x94, 0xab, 0xa6, 0x72, 0x73, 0x37, 0x3e, 0x6c, 0x91, 0x5e, 0x30, 0x7c, 0xae, 0x66, 0x53, 0xf2, 0xb6, 0xa1, 0x09, 0x55, 0x18, 0x60, 0x6f, 0x51, 0xe6, 0x60, 0x3e, 0xd2, 0xc8, 0x69, 0x18, 0x2d, 0xbb, 0xb8, 0xce, 0xd1, 0xb7, 0xe1, 0x26, 0x5f, 0x54, 0x3e, 0x8f, 0xfc, 0xd7, 0x88, 0x4a, 0x96, 0x48, 0x4c, 0x25, 0x82, 0xed, 0xe8, 0x87, 0x7d, 0x17, 0xa2, 0x20, 0xed, 0x69, 0x2d, 0xb5, 0x39, 0xa9, 0x34, 0xcd, 0xf2, 0x4f, 0x1a, 0x6c, 0x21, 0x31, 0x85, 0x58, 0x03, 0xba, 0x5e, 0x77, 0xd2, 0xca, 0xbf, 0x4c, 0x3d, 0x65, 0x14, 0x5e, 0x96, 0xc8, 0x3f, 0x6c, 0xbf, 0x01, 0xc6, 0x7d, 0xca, 0xaf, 0xee, 0x06, 0x00, 0x00};

// ".outlined{border: 1px solid #FFFFFF;margin: 1px;}body {background: black;padding: 0.25em;margin: 0;font-size: 16px;font-family: Verdana, sans-serif;color: white;} html,body {height: 99vh;}table, th, td {border-bottom: 1px solid #ddd;border-collapse: collapse;}th, td {padding: 15px;text-align: left;}tr:hover {background-color: rgb(52, 79, 233);}button {background: #222222;padding: 6.5px 10px;margin: 6.5px 10px;-webkit-border-radius: 3px;-moz-border-radius: 3px;border-radius: 3px;color: white;font-size: 13px;font-family: \"localMichroma\", monospace;text-decoration: none;vertical-align: middle;cursor:pointer; } button:hover {background: #444;color: white; } button:active {background: #333; } button:disabled {background: black;color: #555; } .tab {overflow: hidden;border: 1px solid #555;background-color: #111;} .tab button {background-color: inherit;float: left;border: none;outline: none;cursor: pointer;padding: 14px 16px;transition: 0.3s;font-size: 17px;} .tab button:hover {background-color: #333;} .tab button.active {background-color: #444;} .tabcontent {display: none;padding: 6px 12px;border: 1px solid #222;border-top: none;animation: fadeEffect 0.2s; } @keyframes fadeEffect {from {opacity: 0;}to {opacity: 1;}}.modal {display: none;position: fixed;z-index: 1;padding-top: 100px;left: 0;top: 0;width: 100%;height: 100%;overflow: auto;background-color: rgb(0,0,0);background-color: rgba(0,0,0,0.4);} .modal-content {background-color: #bbbbbb;color: #000;margin: auto;padding: 20px;border: 1px solid #888;width: 80%;} .modal-close {color: #aaaaaa;float: right;font-size: 28px;font-weight: bold;}.modal-close:hover,.modal-close:focus {color: #000;text-decoration: none;cursor: pointer;}"
static const unsigned char main_css[] = {0x1f, 0x8b, 0x08, 0x00, 0xbe, 0x70, 0x63, 0x62, 0x02, 0xff, 0x75, 0x54, 0xc1, 0x6e, 0xa3, 0x30, 0x10, 0xfd, 0x15, 0xab, 0xd1, 0x4a, 0x5b, 0x09, 0x10, 0x09, 0x49, 0x9b, 0xe2, 0xcb, 0x5e, 0x76, 0x6f, 0x7b, 0xdd, 0xbb, 0xc1, 0x26, 0x58, 0x35, 0x9e, 0xc8, 0x76, 0x9a, 0xa4, 0x11, 0xff, 0xbe, 0x63, 0x30, 0x04, 0x1a, 0xea, 0x28, 0x12, 0xf6, 0x8c, 0x67, 0xde, 0xbc, 0x79, 0x9e, 0x04, 0x4e, 0x4e, 0x49, 0x2d, 0xf8, 0xad, 0x00, 0xc3, 0x85, 0xc9, 0xc9, 0xfa, 0x78, 0x21, 0x16, 0x94, 0xe4, 0x64, 0xf5, 0xa7, 0x5b, 0xb4, 0x61, 0xe6, 0x20, 0x75, 0x67, 0xa1, 0x6d, 0x01, 0xfc, 0x4a, 0x6e, 0x05, 0x2b, 0xdf, 0x0f, 0x06, 0x4e, 0x9a, 0xe7, 0xa4, 0x50, 0xb8, 0xa1, 0x47, 0xc6, 0xb9, 0xd4, 0x87, 0x9c, 0xa4, 0xc9, 0x66, 0x27, 0x9a, 0xf1, 0x52, 0x4a, 0x2b, 0xd0, 0x2e, 0xb6, 0xf2, 0x53, 0x60, 0x84, 0x17, 0x0c, 0xd1, 0xed, 0x2b, 0xd6, 0x48, 0x75, 0xcd, 0xc9, 0x3f, 0x61, 0x38, 0xd3, 0x2c, 0x22, 0x96, 0x69, 0x1b, 0x5b, 0x61, 0x64, 0x45, 0x4b, 0x50, 0x80, 0x40, 0xce, 0xb5, 0x74, 0x82, 0xb6, 0xa4, 0x76, 0x8d, 0x8a, 0xfa, 0xb4, 0xb5, 0x90, 0x87, 0xda, 0xe5, 0xe4, 0xed, 0xed, 0xa3, 0xa6, 0xad, 0x63, 0x85, 0x12, 0x11, 0x71, 0x35, 0xfe, 0x39, 0x09, 0x05, 0xc4, 0x05, 0x38, 0x07, 0xcd, 0xac, 0x0e, 0xce, 0x39, 0x0d, 0x56, 0x8c, 0xad, 0xd8, 0xd1, 0x22, 0x96, 0xe1, 0x0b, 0xe3, 0x84, 0x00, 0x63, 0x09, 0xeb, 0x1d, 0xc2, 0x74, 0xe2, 0xe2, 0x62, 0xa6, 0xe4, 0x01, 0x8b, 0x50, 0xa2, 0x72, 0xe8, 0x67, 0xf2, 0x1a, 0x3e, 0x84, 0x99, 0x96, 0x1f, 0x07, 0xb0, 0xe6, 0x50, 0xfc, 0xdc, 0x6d, 0x22, 0xf2, 0xfa, 0x16, 0x91, 0x4d, 0x96, 0x3d, 0x23, 0x51, 0x27, 0xc4, 0xa1, 0xe7, 0x54, 0xad, 0x36, 0xdd, 0xba, 0x93, 0xf5, 0x92, 0x60, 0x2a, 0xb2, 0x4e, 0x31, 0xdf, 0x40, 0xd8, 0xe4, 0x28, 0x3e, 0x8b, 0xe2, 0x5d, 0xba, 0x38, 0x60, 0x37, 0x8c, 0xcb, 0x93, 0xcd, 0x49, 0xe6, 0x4d, 0x0d, 0x7c, 0x2e, 0x9d, 0x2f, 0x1c, 0xcd, 0xe8, 0x9c, 0x36, 0x23, 0xfb, 0xda, 0x8c, 0x27, 0x05, 0x25, 0x53, 0x7f, 0x65, 0x59, 0x1b, 0x68, 0xd8, 0x53, 0x44, 0x1a, 0xd0, 0x60, 0x8f, 0xac, 0x14, 0x3d, 0x1b, 0x5c, 0x94, 0x60, 0x98, 0x93, 0x80, 0x30, 0x35, 0x68, 0x41, 0x91, 0x0c, 0x27, 0xf1, 0xca, 0xc0, 0x53, 0x23, 0x39, 0x57, 0x82, 0x96, 0x27, 0x63, 0x31, 0xe5, 0x11, 0xa4, 0x76, 0xc2, 0x50, 0xd2, 0x92, 0x9e, 0x8c, 0x47, 0xfa, 0x90, 0x92, 0xed, 0x76, 0x3b, 0x87, 0x78, 0x77, 0x67, 0xa5, 0x93, 0x1f, 0xe2, 0x8b, 0x7f, 0x96, 0x65, 0x13, 0x17, 0x2e, 0xad, 0x57, 0x01, 0x5f, 0x92, 0x64, 0x88, 0xba, 0xda, 0xed, 0x76, 0xfe, 0x46, 0x82, 0x7a, 0x21, 0x37, 0x8f, 0xa0, 0x52, 0x70, 0xce, 0x49, 0x8d, 0x60, 0x85, 0xa6, 0x0b, 0xba, 0xf7, 0x17, 0x1e, 0x5b, 0xbc, 0x5a, 0xaf, 0xd7, 0x34, 0x84, 0x79, 0x6c, 0xee, 0xe0, 0x25, 0x75, 0x8d, 0x22, 0x76, 0x14, 0x73, 0x30, 0x17, 0x94, 0x33, 0xa4, 0xe8, 0x38, 0x0b, 0x0f, 0x2e, 0xec, 0x02, 0x55, 0x64, 0xe0, 0xea, 0xae, 0xc1, 0xad, 0x57, 0x81, 0x7f, 0x2f, 0xce, 0xe0, 0xd3, 0x90, 0x3d, 0xeb, 0x69, 0x92, 0xd9, 0x59, 0x0f, 0x5f, 0xfd, 0x9b, 0x9c, 0x62, 0xfa, 0x5e, 0xa2, 0x1d, 0x73, 0x33, 0xdf, 0xe4, 0x91, 0xe0, 0xd1, 0xd9, 0xb7, 0xa5, 0x77, 0x2e, 0x31, 0x9d, 0xd0, 0x8e, 0xdc, 0x90, 0xeb, 0xa3, 0x62, 0xd7, 0x80, 0xfc, 0xae, 0x61, 0x0f, 0x74, 0x33, 0x6a, 0x6f, 0xc6, 0xa4, 0x17, 0x7b, 0x90, 0xa4, 0x83, 0x63, 0xb8, 0xc9, 0xb4, 0x6c, 0x82, 0x8a, 0x2a, 0xc6, 0xc5, 0xef, 0xaa, 0x12, 0xa5, 0xf3, 0x73, 0xc3, 0xfa, 0x36, 0xfd, 0x7a, 0x17, 0xd7, 0xca, 0xb0, 0x46, 0xd8, 0xa9, 0xf5, 0x56, 0xa1, 0x24, 0xb1, 0x7b, 0x28, 0x46, 0xe9, 0xae, 0x7e, 0xac, 0xb4, 0x0e, 0x26, 0x7b, 0xec, 0x4c, 0x9b, 0x34, 0xc0, 0x99, 0x7a, 0xc0, 0x09, 0x03, 0x79, 0x95, 0xbc, 0x08, 0x4e, 0x3f, 0x63, 0xa9, 0xb9, 0xb8, 0xf8, 0x2b, 0xa1, 0x84, 0x1e, 0xda, 0x3a, 0xf5, 0x4f, 0xce, 0xf7, 0xcb, 0x47, 0xef, 0x8e, 0x52, 0x7a, 0x96, 0xdc, 0xd5, 0x9d, 0xed, 0x07, 0x1d, 0x86, 0x4f, 0xb7, 0xb9, 0xcb, 0x88, 0x9d, 0x1c, 0xd0, 0xe5, 0x81, 0x90, 0x46, 0xf8, 0x7b, 0x5e, 0x36, 0xb2, 0xde, 0x1a, 0xa5, 0xc9, 0xf6, 0xd9, 0x13, 0xdd, 0x61, 0x8f, 0x47, 0xae, 0x17, 0x3a, 0x52, 0x74, 0x6b, 0x54, 0x75, 0x9a, 0xa6, 0xe3, 0xcc, 0xe8, 0x30, 0x8c, 0x0d, 0xd9, 0xa4, 0xcb, 0xcd, 0xd8, 0xef, 0xf7, 0x43, 0x41, 0x7b, 0x2c, 0xe1, 0x9e, 0x54, 0x81, 0x45, 0x11, 0x0c, 0x81, 0x59, 0xb7, 0x06, 0x01, 0x1b, 0x5f, 0xf4, 0x54, 0x72, 0x9b, 0xfd, 0x30, 0x36, 0xce, 0x81, 0x90, 0x02, 0x14, 0xa7, 0xed, 0x34, 0x58, 0x2f, 0xc2, 0x68, 0x76, 0x54, 0x41, 0x79, 0xb2, 0xf7, 0x2c, 0x1e, 0xfe, 0xf2, 0x50, 0xf9, 0xfa, 0x24, 0xda, 0xff, 0x05, 0xca, 0x61, 0x6b, 0xa4, 0x06, 0x00, 0x00};

// "getCurSettings();websocketConnect();"
static const unsigned char main_js[] = {0x1f, 0x8b, 0x08, 0x00, 0xbe, 0x70, 0x63, 0x62, 0x02, 0xff, 0x4b, 0x4f, 0x2d, 0x71, 0x2e, 0x2d, 0x0a, 0x4e, 0x2d, 0x29, 0xc9, 0xcc, 0x4b, 0x2f, 0xd6, 0xd0, 0xb4, 0x2e, 0x4f, 0x4d, 0x2a, 0xce, 0x4f, 0xce, 0x06, 0x0a, 0xe7, 0xe7, 0xe5, 0xa5, 0x26, 0x97, 0x00, 0x85, 0x00, 0x61, 0x45, 0x30, 0x16, 0x24, 0x00, 0x00, 0x00};

// "var modal=document.getElementById(\"myModal\");var span=document.getElementsByClassName(\"close\")[0];function showModal(message){document.getElementById(\"modalText\").innerHTML=message;modal.style.display=\"block\";}\nfunction closeModal(){modal.style.display=\"none\";document.getElementById(\"modalText\").innerHTML=\"\";}\nspan.onclick=function(){closeModal();}\nwindow.onclick=function(event){if(event.target==modal){closeModal();}}"
static const unsigned char modal_js[] = {0x1f, 0x8b, 0x08, 0x00, 0xbe, 0x70, 0x63, 0x62, 0x02, 0xff, 0x95, 0x90, 0xb1, 0x4e, 0x03, 0x31, 0x0c, 0x86, 0x77, 0x9e, 0xa2, 0xf2, 0x74, 0x59, 0x22, 0xf6, 0x28, 0x4b, 0x11, 0x12, 0x48, 0x94, 0xa9, 0x1b, 0xea, 0x10, 0x12, 0xb7, 0x44, 0x4d, 0x9c, 0xea, 0x9c, 0xde, 0x35, 0x3a, 0xf5, 0xdd, 0x49, 0xae, 0x80, 0x10, 0x94, 0x81, 0xcd, 0x92, 0xbf, 0xdf, 0xdf, 0x2f, 0x0f, 0xa6, 0x5f, 0xc4, 0xe4, 0x4c, 0xd0, 0x2e, 0xd9, 0x63, 0x44, 0xca, 0x72, 0x87, 0xf9, 0x3e, 0x60, 0x1b, 0x97, 0xe5, 0xd1, 0x75, 0x10, 0xcb, 0xaa, 0x01, 0x20, 0xd4, 0x50, 0x61, 0x3e, 0x18, 0xba, 0xc6, 0xf2, 0xb2, 0xdc, 0x05, 0xc3, 0xfc, 0x6c, 0x22, 0x76, 0x60, 0x43, 0x62, 0x04, 0xf1, 0x72, 0xbb, 0x51, 0xdb, 0x23, 0xd9, 0xec, 0x13, 0x2d, 0xf8, 0x2d, 0x8d, 0xf3, 0xa5, 0x2e, 0x22, 0xb3, 0xd9, 0xa1, 0x98, 0xfe, 0x76, 0x36, 0x6e, 0x8d, 0xa7, 0x0c, 0x42, 0x7a, 0x22, 0xec, 0x1f, 0xd6, 0xab, 0x27, 0xfd, 0x91, 0x53, 0xf3, 0x56, 0x72, 0x2e, 0x01, 0xa5, 0xf3, 0x7c, 0x08, 0xa6, 0x68, 0x78, 0x0d, 0xc9, 0xee, 0x41, 0x9d, 0x6f, 0xbe, 0x84, 0x73, 0x89, 0x8b, 0x51, 0x4c, 0x57, 0x33, 0x94, 0x08, 0x41, 0xfd, 0xb3, 0x05, 0x34, 0x49, 0x7b, 0x83, 0x4c, 0x64, 0x83, 0xb7, 0x7b, 0xfd, 0x69, 0xac, 0x9a, 0xef, 0xce, 0x8a, 0x8d, 0x9e, 0x5c, 0x1a, 0x7f, 0x83, 0x38, 0x54, 0x89, 0x98, 0xfc, 0xf6, 0x32, 0xc9, 0x6c, 0xfa, 0x2a, 0xd7, 0x7a, 0x36, 0xfe, 0xbc, 0x72, 0x7e, 0x07, 0xd7, 0x64, 0x66, 0x46, 0xa5, 0x01, 0x00, 0x00};

// "function onSettingsReset(evt){getCurSettings();}\nfunction onSettingsSubmit(evt){txData={}\ntxData.deviceName=document.getElementById(\"deviceName\").value;txData.ucIPAddress=document.getElementById(\"ucIPAddress\").value;txData.ucNetMask=document.getElementById(\"ucNetMask\").value;txData.ucGatewayAddress=document.getElementById(\"ucGatewayAddress\").value;txData.nt4ServerAddress=document.getElementById(\"nt4ServerAddress\").value;newSettings=JSON.stringify(txData);sendNewSettings(newSettings);}\ncurSettings_url=\"http://\"+window.location.hostname+\":\"+window.location.port+\"/curSettings\"\nfunction getCurSettings(){showModal(\"Retrieving Settings...\")\nvar xmlHttp=new XMLHttpRequest();xmlHttp.onreadystatechange=function(){if(xmlHttp.readyState==4&&xmlHttp.status==200){onRxCurSettings(xmlHttp.responseText);}}\nxmlHttp.open(\"GET\",curSettings_url,true);xmlHttp.send(null);}\nnewSettings_url=\"http://\"+window.location.hostname+\":\"+window.location.port+\"/newSettings\"\nfunction sendNewSettings(newSettings){showModal(\"Updating Settings...\")\nvar xmlHttp=new XMLHttpRequest();xmlHttp.onreadystatechange=function(){if(xmlHttp.readyState==4){if(xmlHttp.status==200){showModal(\"Settings successfully updated!\");setTimeout(window.location.reload.bind(window.location),1500);}else{showModal(\"ERROR, failed to update settings!\");}}}\nxmlHttp.open(\"POST\",newSettings_url,true);xmlHttp.send(newSettings);}\nfunction onRxCurSettings(settings){rxData=JSON.parse(settings);document.getElementById(\"deviceName\").value=rxData.deviceName;document.getElementById(\"ucIPAddress\").value=rxData.ucIPAddress;document.getElementById(\"ucNetMask\").value=rxData.ucNetMask;document.getElementById(\"ucGatewayAddress\").value=rxData.ucGatewayAddress;document.getElementById(\"nt4ServerAddress\").value=rxData.nt4ServerAddress;window.document.title=rxData.deviceName;document.getElementById(\"deviceTitleText\").innerHTML=rxData.deviceName;closeModal();}\nfunction validateDeviceName(str_in){var devNameRegex=new RegExp(\"[a-zA-Z_]*\");return devNameRegex.test(str_in);}\nfunction validateIpAddr(str_in){var ipRegex=new RegExp(\"^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$\");return ipRegex.test(str_in);}"
static const unsigned char settings_js[] = {0x1f, 0x8b, 0x08, 0x00, 0xbe, 0x70, 0x63, 0x62, 0x02, 0xff, 0xbd, 0x54, 0xdb, 0x4e, 0xdb, 0x40, 0x10, 0x7d, 0xcf, 0x57, 0xb8, 0x56, 0x85, 0xec, 0x12, 0x4c, 0xa0, 0xf0, 0x50, 0x2c, 0x0b, 0xd1, 0x12, 0x01, 0x15, 0x09, 0xc8, 0x49, 0xa5, 0xaa, 0x34, 0x45, 0x8b, 0x3d, 0x49, 0xac, 0x3a, 0xbb, 0x66, 0x77, 0x36, 0x97, 0x86, 0xfc, 0x7b, 0x77, 0x13, 0x3b, 0xbe, 0x84, 0x52, 0x22, 0x55, 0x95, 0x22, 0x67, 0xed, 0x39, 0x33, 0x67, 0x74, 0xce, 0xec, 0xf4, 0x25, 0x0d, 0x30, 0x62, 0xd4, 0x60, 0xb4, 0x03, 0x88, 0x11, 0x1d, 0x08, 0x1f, 0x04, 0xa0, 0x05, 0x63, 0xb4, 0xe7, 0x03, 0xc0, 0x4f, 0x92, 0x67, 0x01, 0xcb, 0x76, 0x17, 0xb5, 0xfe, 0x66, 0x42, 0x47, 0x3e, 0x8c, 0xa2, 0x34, 0x03, 0xa7, 0xe7, 0x04, 0x89, 0x37, 0x5f, 0xd4, 0x56, 0x27, 0x27, 0x84, 0x71, 0x14, 0x40, 0x9b, 0x8c, 0xc0, 0x0b, 0x59, 0x20, 0x47, 0x40, 0xd1, 0x51, 0x65, 0x9b, 0x31, 0xe8, 0xe3, 0xc7, 0xd9, 0x55, 0x68, 0x99, 0x39, 0xc6, 0xb4, 0x9d, 0x31, 0x89, 0x25, 0xb8, 0x69, 0xb6, 0x0c, 0xae, 0x6e, 0xcf, 0xc2, 0x90, 0x83, 0x10, 0x7f, 0x4e, 0x2f, 0x80, 0x36, 0xf3, 0xdb, 0x80, 0x2d, 0x22, 0x7e, 0xbe, 0x94, 0x9d, 0x42, 0x36, 0x73, 0x2f, 0x08, 0xc2, 0x84, 0xcc, 0x5e, 0xd1, 0x40, 0x19, 0x59, 0xad, 0x44, 0xf1, 0xa8, 0x03, 0x7c, 0x0c, 0xfc, 0xaf, 0x95, 0xaa, 0xc8, 0x75, 0x25, 0x0a, 0x93, 0x4c, 0x6e, 0xef, 0x73, 0xe7, 0xa6, 0xed, 0x08, 0xe4, 0xea, 0x25, 0xea, 0xcf, 0xac, 0x15, 0x89, 0xed, 0x0a, 0xa0, 0x61, 0x3b, 0x87, 0x59, 0x85, 0x14, 0x6d, 0x5c, 0x90, 0x1b, 0x79, 0x2f, 0x79, 0xec, 0x99, 0x43, 0xc4, 0xe4, 0x64, 0x7f, 0xdf, 0xdc, 0x9d, 0x44, 0x34, 0x64, 0x13, 0x27, 0x66, 0x01, 0xd1, 0xce, 0x3a, 0x43, 0x26, 0x90, 0x2a, 0x33, 0x76, 0xcd, 0x93, 0xcd, 0x60, 0xc2, 0x38, 0xee, 0x9a, 0xfb, 0x85, 0x6a, 0x66, 0x3e, 0x13, 0xd5, 0x79, 0x99, 0x8b, 0x21, 0x9b, 0xb4, 0x58, 0x48, 0x62, 0xcb, 0xf4, 0x41, 0x75, 0xac, 0x8c, 0xa6, 0x03, 0x23, 0x03, 0x38, 0x8e, 0x63, 0xda, 0xb5, 0x31, 0xe1, 0xc6, 0x74, 0x14, 0x5f, 0xaa, 0x76, 0x3c, 0xd5, 0xb3, 0xf1, 0xb5, 0x75, 0xad, 0xcf, 0x3e, 0x3c, 0x4a, 0x10, 0xa8, 0x86, 0x2e, 0x0d, 0x3a, 0x8c, 0x72, 0x20, 0xe1, 0x4c, 0xa0, 0x12, 0x3b, 0x18, 0x12, 0x3a, 0x00, 0x2f, 0x63, 0x56, 0x54, 0x51, 0xdf, 0xca, 0x80, 0x4b, 0x58, 0x47, 0xc3, 0x3c, 0xef, 0x68, 0x67, 0x27, 0xfb, 0xac, 0x13, 0xa5, 0xf0, 0xbc, 0xc3, 0x46, 0xc3, 0x9e, 0x33, 0xea, 0x4f, 0x8b, 0xbd, 0xe6, 0xb9, 0x22, 0x61, 0x54, 0x40, 0x17, 0xa6, 0xa8, 0x64, 0x5b, 0xd4, 0xd6, 0xec, 0x09, 0x50, 0xcb, 0xbc, 0x68, 0x76, 0xcd, 0x7a, 0x45, 0xca, 0x3a, 0x72, 0x09, 0x79, 0x9b, 0xda, 0x07, 0x8b, 0xca, 0x38, 0xd6, 0xaa, 0x17, 0x4c, 0xf8, 0x07, 0xaa, 0x17, 0xaa, 0x15, 0x54, 0x7f, 0xc9, 0xf8, 0xa2, 0x03, 0x5f, 0x92, 0x90, 0xe0, 0x7f, 0xd7, 0xbf, 0x14, 0x29, 0x59, 0x50, 0x68, 0x2d, 0xeb, 0xc8, 0x10, 0x32, 0x08, 0xd4, 0xdc, 0xf7, 0x95, 0x7c, 0x33, 0x43, 0xea, 0x86, 0x21, 0x7c, 0x63, 0xea, 0xe1, 0xc6, 0x6e, 0x34, 0x02, 0x26, 0xd1, 0xaa, 0x6a, 0xc3, 0x21, 0x66, 0x24, 0x74, 0x1e, 0xd4, 0xe7, 0x6a, 0xcc, 0xae, 0x1f, 0x1c, 0x2b, 0x26, 0x77, 0x01, 0xb1, 0x80, 0x22, 0x5f, 0xd3, 0xf7, 0x6f, 0xfc, 0xba, 0xd1, 0x27, 0x51, 0x0c, 0xa1, 0x81, 0x2c, 0xa5, 0x52, 0x52, 0xae, 0xfa, 0xd0, 0x94, 0x8b, 0x0d, 0xf3, 0x6f, 0x6f, 0x3a, 0xca, 0xfd, 0x8a, 0xa5, 0xcf, 0xba, 0x5f, 0xbe, 0x7a, 0x85, 0x9d, 0x59, 0x1e, 0x3a, 0xb1, 0xb6, 0x89, 0xaf, 0xf6, 0xe6, 0xf2, 0x6a, 0x27, 0x84, 0x0b, 0xc8, 0x63, 0xee, 0x16, 0x7b, 0xd3, 0xe3, 0xd5, 0xad, 0xeb, 0x6e, 0xb3, 0x36, 0xb3, 0xf4, 0x42, 0xc8, 0x7d, 0xfd, 0xe2, 0xcc, 0xb3, 0xd3, 0x80, 0xbb, 0xf5, 0xc6, 0xcc, 0x4b, 0x94, 0xe3, 0xee, 0xd6, 0x1b, 0x33, 0xab, 0x54, 0x8d, 0xbb, 0xe9, 0x88, 0xac, 0x0b, 0x62, 0x84, 0xf1, 0x56, 0xba, 0xad, 0x30, 0x5d, 0x9d, 0xa6, 0x97, 0x84, 0x22, 0x8c, 0x28, 0x05, 0x7e, 0xd9, 0x6d, 0x5d, 0x3f, 0x53, 0x26, 0x88, 0x99, 0x80, 0xd5, 0xd4, 0x95, 0x46, 0x41, 0x35, 0x19, 0xe9, 0x91, 0x3b, 0x5f, 0x43, 0x2d, 0xb5, 0xd2, 0xef, 0x23, 0x6a, 0xcf, 0xf5, 0x85, 0x54, 0x15, 0xf4, 0x37, 0x1f, 0x06, 0x30, 0x5d, 0xde, 0x4a, 0x75, 0x6a, 0x4e, 0x13, 0xcb, 0xbc, 0x23, 0x7b, 0xbf, 0xce, 0xf6, 0xbe, 0xdd, 0xf7, 0xde, 0xa9, 0x11, 0xe5, 0x80, 0x92, 0xd3, 0x12, 0xd8, 0x41, 0x7d, 0x69, 0xd3, 0x52, 0xcf, 0x11, 0x5e, 0x25, 0x5a, 0x89, 0x12, 0x59, 0x94, 0x6c, 0xf2, 0xfc, 0xb0, 0x4e, 0x4f, 0xd4, 0xef, 0xf0, 0xf8, 0xae, 0xb1, 0x77, 0xdc, 0x7b, 0x3a, 0x54, 0x7f, 0x47, 0x3d, 0xf5, 0xf8, 0xd0, 0x7b, 0xba, 0x6b, 0x1c, 0xf4, 0x4e, 0x97, 0xc7, 0xe5, 0xe3, 0xd4, 0xfe, 0xee, 0xd8, 0xf3, 0xf7, 0x8b, 0xd7, 0xa2, 0xdf, 0xe6, 0x8d, 0xa7, 0xc4, 0x95, 0x9e, 0x7f, 0x03, 0x73, 0x95, 0x58, 0xa0, 0x94, 0x08, 0x00, 0x00};

// "function selectTab(evt,tabName){var i,tabcontent,tablinks;tabcontent=document.getElementsByClassName(\"tabcontent\");for(i=0;i<tabcontent.length;i++){tabcontent[i].style.display=\"none\";}\ntablinks=document.getElementsByClassName(\"tablinks\");for(i=0;i<tablinks.length;i++){tablinks[i].className=tablinks[i].className.replace(\" active\",\"\");}\ndocument.getElementById(tabName).style.display=\"block\";evt.currentTarget.className+=\" active\";}\ndocument.getElementById(\"defaultOpen\").click();"
static const unsigned char tabs_js[] = {0x1f, 0x8b, 0x08, 0x00, 0xbe, 0x70, 0x63, 0x62, 0x02, 0xff, 0x8d, 0x90, 0x31, 0x6b, 0xc3, 0x30, 0x10, 0x85, 0xf7, 0xfc, 0x0a, 0xa3, 0x49, 0xc2, 0x41, 0x74, 0x57, 0xb5, 0xa4, 0x74, 0xe8, 0xd2, 0x2e, 0xd9, 0x42, 0x06, 0xf9, 0x7c, 0x4e, 0x85, 0x15, 0x29, 0x48, 0xb2, 0xc1, 0x84, 0xfc, 0xf7, 0x9e, 0x0d, 0xae, 0xc0, 0x84, 0xd2, 0xed, 0xf4, 0xee, 0xee, 0x7d, 0xa7, 0xd7, 0x0d, 0x1e, 0xb2, 0x0d, 0xbe, 0x4a, 0xe8, 0x10, 0xf2, 0xd1, 0x34, 0x1c, 0xc7, 0xbc, 0xcf, 0xa6, 0xf9, 0x34, 0x57, 0x14, 0xf7, 0xd1, 0xc4, 0xca, 0xce, 0x4f, 0x08, 0x3e, 0xa3, 0x5f, 0x3a, 0xce, 0xfa, 0x3e, 0xa9, 0xa2, 0xe9, 0x36, 0xc0, 0x70, 0xa5, 0x42, 0x5e, 0x30, 0xbf, 0x3b, 0x9c, 0xcb, 0x74, 0x98, 0xde, 0x9c, 0x49, 0x69, 0x76, 0xe1, 0xac, 0xcc, 0x32, 0xa1, 0xba, 0x10, 0xb9, 0xd5, 0x2f, 0xca, 0xbe, 0x16, 0x59, 0x3a, 0xf4, 0x97, 0xfc, 0xad, 0x6c, 0x5d, 0x8b, 0x7b, 0x91, 0x4f, 0xf6, 0x2c, 0x53, 0x9e, 0x1c, 0xca, 0xd6, 0xa6, 0x9b, 0x33, 0x93, 0x66, 0x3e, 0x78, 0x64, 0xea, 0xb1, 0x5b, 0xef, 0xf8, 0x17, 0x7c, 0x99, 0xdc, 0xa2, 0x17, 0x71, 0x0b, 0x5e, 0xc4, 0x19, 0x0b, 0xab, 0x81, 0x7e, 0xaa, 0xca, 0x88, 0x74, 0x0f, 0x90, 0x7d, 0x65, 0x28, 0xc0, 0x11, 0xd9, 0x9e, 0x11, 0xe0, 0xb1, 0x7b, 0x72, 0xce, 0x61, 0xfa, 0x68, 0xf9, 0x9a, 0xe8, 0xf6, 0x3f, 0x8d, 0x0b, 0xd0, 0x33, 0x45, 0xa1, 0x4b, 0x18, 0x62, 0xa4, 0xf1, 0xa3, 0x89, 0xb4, 0x5b, 0x50, 0xb5, 0xfe, 0x65, 0xfc, 0xe1, 0xcf, 0x5a, 0xec, 0xcc, 0xe0, 0xf2, 0xd7, 0x0d, 0x3d, 0x13, 0xb4, 0x6d, 0xa1, 0xe7, 0x42, 0xfd, 0x00, 0x0e, 0x4c, 0x70, 0xd4, 0xe0, 0x01, 0x00, 0x00};

// "ws_url=\"ws://\"+window.location.hostname+\":\"+window.location.port+\"/websocket\"\nwebSocket=null;function websocketConnect(){webSocket=new WebSocket(ws_url);webSocket.onopen=wsOnOpenHandler;webSocket.onmessage=wsOnMessageHandler;}\nfunction wsOnOpenHandler(event){};function wsOnMessageHandler(event){rxData=JSON.parse(event.data);if(rxData.msgType==\"newData\"){var dataTable=document.getElementById(\"dataTable\");for(key in rxData){value=rxData[key];var row=document.getElementById(key);if(!row){row=dataTable.insertRow(-1);row.id=key;row.insertCell(0);row.insertCell(1);}\nrow.cells[0].innerHTML=key;row.cells[1].innerHTML=rxData[key];}}else if(rxData.msgType==\"curSettings\"){}else{console.log(\"Unknown message type \"+rxData.msgType);}}"
static const unsigned char websocketData_js[] = {0x1f, 0x8b, 0x08, 0x00, 0xbe, 0x70, 0x63, 0x62, 0x02, 0xff, 0x75, 0x91, 0x41, 0x4f, 0xe3, 0x30, 0x10, 0x85, 0xef, 0xfd, 0x15, 0x5e, 0x9f, 0x1c, 0x55, 0x9b, 0xc2, 0x95, 0xc8, 0x97, 0x65, 0x91, 0x00, 0x01, 0x95, 0x68, 0x57, 0x7b, 0x40, 0x08, 0x99, 0x64, 0x1a, 0xa2, 0xba, 0x33, 0x95, 0xed, 0xe0, 0xad, 0xaa, 0xfc, 0xf7, 0x1d, 0x27, 0xdb, 0xd2, 0xb0, 0xbb, 0xb7, 0xe7, 0x79, 0xdf, 0xbc, 0x3c, 0x3b, 0xd1, 0xbf, 0xb4, 0xce, 0x6a, 0x19, 0xfd, 0xc5, 0x6c, 0x26, 0xa7, 0xb1, 0xc1, 0x8a, 0x62, 0x6e, 0xa9, 0x34, 0xa1, 0x21, 0xcc, 0xdf, 0xc8, 0x07, 0x34, 0x1b, 0x98, 0xca, 0x8b, 0xbf, 0xcd, 0x2d, 0xb9, 0x30, 0x95, 0xb3, 0x08, 0xaf, 0x9e, 0xca, 0x35, 0x04, 0x39, 0x61, 0xb9, 0xe8, 0xa5, 0xc6, 0xd6, 0xda, 0x62, 0xd5, 0x62, 0x99, 0x48, 0x71, 0x44, 0x2e, 0x09, 0x11, 0xca, 0xa0, 0xb2, 0xfd, 0x09, 0x0a, 0x51, 0xfc, 0x3c, 0x9c, 0x54, 0xec, 0x0b, 0x65, 0xc5, 0xd1, 0xcf, 0x09, 0x69, 0x0b, 0xa8, 0xa3, 0x9f, 0xe3, 0x9c, 0xc5, 0xb5, 0xc1, 0xca, 0x82, 0x1b, 0x01, 0x1b, 0xf0, 0xde, 0xd4, 0xd0, 0x33, 0xf7, 0x83, 0x3e, 0x60, 0xdd, 0xe4, 0xa3, 0xc5, 0x38, 0x41, 0xc1, 0x3b, 0x60, 0xc8, 0xf6, 0x5d, 0x31, 0x22, 0xc6, 0xfb, 0x07, 0xc8, 0xfd, 0xfa, 0x6e, 0x82, 0xd1, 0xb7, 0x8b, 0xf9, 0x43, 0xbe, 0x35, 0xce, 0xc3, 0x60, 0xe4, 0x15, 0x4f, 0xb3, 0xa2, 0x59, 0xa9, 0x01, 0xc8, 0x37, 0xbe, 0x5e, 0xee, 0xb6, 0xa0, 0xb5, 0xe4, 0x6b, 0xa5, 0x89, 0xcc, 0xf6, 0xef, 0xc6, 0x89, 0xc4, 0x2d, 0xcd, 0xab, 0x05, 0x5d, 0x51, 0xd9, 0x6e, 0xd2, 0x6a, 0x0d, 0xe1, 0xca, 0x42, 0x92, 0xdf, 0x76, 0x37, 0x95, 0x92, 0x47, 0x44, 0x66, 0xc5, 0x8a, 0x9c, 0x5a, 0xc3, 0x4e, 0x34, 0x28, 0x86, 0xe0, 0x94, 0x62, 0x5b, 0xd0, 0xc3, 0xe9, 0x89, 0xbd, 0xe7, 0x22, 0xe5, 0x3a, 0x8a, 0xff, 0x4d, 0x64, 0xa8, 0xaf, 0xf6, 0x85, 0x21, 0xbe, 0x41, 0x22, 0x0f, 0x9f, 0xc8, 0x1b, 0xf4, 0xe0, 0xc2, 0x23, 0x45, 0xf5, 0xf5, 0x3c, 0x2b, 0xd8, 0xcb, 0x9b, 0x4a, 0xf3, 0xc2, 0x20, 0x7b, 0xf3, 0x12, 0xac, 0x55, 0x67, 0xd9, 0xe7, 0x09, 0xe3, 0xdd, 0x24, 0xcd, 0x4a, 0x3e, 0xf9, 0xa7, 0xb3, 0x67, 0x36, 0x11, 0xdc, 0xf5, 0xf2, 0xfe, 0xee, 0x18, 0x30, 0x58, 0xe7, 0xa7, 0xd6, 0x69, 0xf1, 0xae, 0x03, 0xeb, 0x41, 0xfc, 0xeb, 0xd5, 0xca, 0xd6, 0x2d, 0x20, 0x84, 0x06, 0x6b, 0xcf, 0x2f, 0xd7, 0x73, 0xfb, 0x92, 0xd0, 0x13, 0x77, 0xb6, 0x54, 0x2b, 0xf9, 0x03, 0xd7, 0x48, 0x11, 0xc5, 0x9f, 0x5f, 0x2e, 0x02, 0xef, 0x09, 0x39, 0x1d, 0x07, 0x71, 0xc5, 0xee, 0x37, 0x82, 0xfd, 0x84, 0x34, 0xda, 0x02, 0x00, 0x00};



// Auto-Generated page response handler.
// Returns poiner to requested content, or null if not available.
void handleHttpFileServe(struct mg_connection *c, struct mg_http_message * hm)
{

   if(mg_http_match_uri(hm, "/")) {
      gzip_http_reply(c, 200,  header_html,  index_html, sizeof(index_html) );
      printf("[WEBSERVER] Served /\n");
   } else if(mg_http_match_uri(hm, "/FourOhFour.html")) {
      gzip_http_reply(c, 200,  header_html,  FourOhFour_html , sizeof(FourOhFour_html) );
      printf("[WEBSERVER] Served /FourOhFour.html\n");
   } else if(mg_http_match_uri(hm, "/index.html")) {
      gzip_http_reply(c, 200,  header_html,  index_html , sizeof(index_html) );
      printf("[WEBSERVER] Served /index.html\n");
   } else if(mg_http_match_uri(hm, "/main.css")) {
      gzip_http_reply(c, 200,  header_css,  main_css , sizeof(main_css) );
      printf("[WEBSERVER] Served /main.css\n");
   } else if(mg_http_match_uri(hm, "/main.js")) {
      gzip_http_reply(c, 200,  header_js,  main_js , sizeof(main_js) );
      printf("[WEBSERVER] Served /main.js\n");
   } else if(mg_http_match_uri(hm, "/modal.js")) {
      gzip_http_reply(c, 200,  header_js,  modal_js , sizeof(modal_js) );
      printf("[WEBSERVER] Served /modal.js\n");
   } else if(mg_http_match_uri(hm, "/settings.js")) {
      gzip_http_reply(c, 200,  header_js,  settings_js , sizeof(settings_js) );
      printf("[WEBSERVER] Served /settings.js\n");
   } else if(mg_http_match_uri(hm, "/tabs.js")) {
      gzip_http_reply(c, 200,  header_js,  tabs_js , sizeof(tabs_js) );
      printf("[WEBSERVER] Served /tabs.js\n");
   } else if(mg_http_match_uri(hm, "/websocketData.js")) {
      gzip_http_reply(c, 200,  header_js,  websocketData_js , sizeof(websocketData_js) );
      printf("[WEBSERVER] Served /websocketData.js\n");
   } else {
      //URL Not found
      printf("[WEBSERVER] Could not find requested resource!\n");
      gzip_http_reply(c, 404,  header_html,  FourOhFour_html, sizeof(FourOhFour_html) );
   } 



}