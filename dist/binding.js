"use strict";
const addon = require('../build/Release/photonapi-native');
module.exports.Init = addon.Init;
module.exports.Uninit = addon.Uninit;
module.exports.Connect = addon.Connect;
module.exports.Disconnect = addon.Disconnect;
module.exports.Search = addon.Search;
module.exports.GetThumbnail = addon.GetThumbnail;
