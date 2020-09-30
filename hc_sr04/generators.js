Blockly.JavaScript['kidultra_distance'] = function (block) {
    var code = 'DEV_I2C1.HC_SR04(0, 0x57).get_distance()';
    return [code, Blockly.JavaScript.ORDER_NONE];
};
