# This is the xml parser. It reads the config file and get whatever parameter
# requested.
# By B.Gao Feb. 2019


import xml.etree.ElementTree as et

class xml_parser:
    def __init__(self, f_name):
        self.OK = True
        try:
            self._parse(f_name)
        except:
            self.OK = False

    def _parse(self, f_name):
        # we parse only the GUI controler part, this part of the code is
        # similar to the function _parse_ctl in config.py
        root = et.parse(f_name).getroot().find('GUI_ctl')
        # the advance settings (if any)
        self.conf = {}
        for adv_conf in root.findall('advanced_ctl'):
            name = adv_conf.find('name').text
            val  = adv_conf.find('value').text
            self.conf[name] = val

    def get_par(self, name):
        if name in self.conf:
            # all parameters are in integer format
            return int(self.conf[name])
        return None



