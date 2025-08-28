.. zephyr:code-sample:: tributech_swiot1l_mqtt
   :name: Tributech SWIOT1l MQTT

   Implements Tributech SWIOT1l MQTT.

Overview
********

Sample that can be used with Tributech SWIOT1l MQTT.

Building and Running
********************

This configuration can be built and executed on QEMU as follows:

.. zephyr-app-commands::
   :zephyr-app: samples/tributech_swiot1l_mqtt
   :host-os: unix
   :board: qemu_riscv32
   :goals: run
   :compact:

To build for another board, change "qemu_riscv32" above to that board's name.

Sample Output
=============

.. code-block:: console

   [0;32m*** Booting Zephyr OS build v3.4.99-1783-gf5f3e4f2b6d2 [0m
   [00:00:00.000,000] <inf> main: Starting MQTT sample
   [00:00:00.000,000] <inf> main: Connecting to Wi-Fi SSID my_ssid
   [00:00:02.000,000] <inf> wifi: Connected
   [00:00:02.000,000] <inf> wifi: Got IP address

Exit QEMU by pressing :kbd:`CTRL+C`
