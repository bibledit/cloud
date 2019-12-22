#!/usr/bin/env python3

from selenium import webdriver
import time
 
driver1 = webdriver.Firefox()
driver1.implicitly_wait(2)
driver1.set_window_position(30, 30)
driver1.set_window_size(800,600)

driver2 = webdriver.Firefox()
driver2.implicitly_wait(2)
driver2.set_window_position(900, 30)
driver2.set_window_size(800,600)

driver1.get("https://bibledit.org:8081/editone/index")
driver2.get("https://bibledit.org:8081/editone/index")

input("Do your thing in the browsers and press Enter to continue")

driver1.switch_to.active_element.send_keys("abc")
time.sleep(0.5)
driver2.switch_to.active_element.send_keys("xyz")
