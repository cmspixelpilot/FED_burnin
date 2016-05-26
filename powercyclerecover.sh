# . setup.sh
bin/fpgaconfig -c settings/fw_trkfec.xml -i fc7_tkfec_rarp_en.bit ;
bin/fpgaconfig -c settings/fw_pxfec.xml -i fc7_top016.bit ;
bin/fpgaconfig -c settings/fw_fed03.xml -i fc7_top_v12.bit ;
bin/fpgaconfig -c settings/fw_fed02.xml -i v5.2_16_04_20_RARP_ON.bit

