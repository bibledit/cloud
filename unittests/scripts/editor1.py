#!/usr/bin/env python3

from selenium import webdriver
import time
import os

input("Start a client at localhost:8080 and press Enter to continue")

driver1 = webdriver.Firefox()
driver1.implicitly_wait(2)
driver1.set_window_position(30, 30)
driver1.set_window_size(900,900)

driver2 = webdriver.Firefox()
driver2.implicitly_wait(2)
driver2.set_window_position(1100, 30)
driver2.set_window_size(900,900)

driver1.get("http://localhost:8080/editone/index")
driver2.get("http://localhost:8080/editone/index")

input("Navigate to the Sample Bible Genesis 1:1 and press Enter to continue")

while True:
    print ("Type abc in first browser")
    driver1.switch_to.active_element.send_keys("abc")
    print ("Waiting shortly")
    time.sleep(0.5)
    print ("Type xyz in second browser")
    driver2.switch_to.active_element.send_keys("xyz")
    input("Press Enter to go through the cycle again")

