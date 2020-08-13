from selenium import webdriver
from selenium.webdriver.chrome.options import Options
import cv2 as cv
from selenium import webdriver
from selenium.webdriver.chrome.options import Options
import time
import urllib.request
import random
import json
from bs4 import BeautifulSoup
from urllib.request import urlretrieve
       # 判断像素是否相同


class TouTiao:
    def __init__(self):
        self.cookies = None
        self.chrome_options = Options()
        # self.chrome_options.add_argument('--headless')
        # self.chrome_options.add_argument('--disable-gpu')
        self.browser = webdriver.Chrome('C:\Program Files (x86)\Google\Chrome\Application\chromedriver.exe', options=self.chrome_options)
    def set_cookies(self):
        with open('cookies.json', 'r') as f:
            self.cookies = json.loads(f.read())
        for cookie in self.cookies:
            self.browser.add_cookie(cookie)

    def create_session(self):
        self.browser.get("https://mp.toutiao.com")
        if self.cookies is None:
            self.set_cookies()
        time.sleep(1)
        self.browser.get(
            "https://mp.toutiao.com/profile_v4/analysis/works-single/small_video")

    def login(self):
        self.browser.get(
            "https://mp.toutiao.com/profile_v4/analysis/works-single/small_video")
        self.browser.implicitly_wait(5)
        # 输入手机号
        phone = self.browser.find_element_by_css_selector(
            '#sso_container > div > div.content-main > form > div:nth-child(1) > div.input-container > input')
        phone.send_keys('18811538807')
        # 获取验证码
        self.browser.find_element_by_id('sso_code').click()
        time.sleep(3)
        try:
            # 手动滑块验证
            pass
        except Exception as identifier:
            pass
        verfiy_code_input = input("请输入验证码:")
        # 验证码输入框
        mobile_code = self.browser.find_element_by_css_selector(
            '#sso_container > div > div.content-main > form > div.form-input.form-input-last > input')
        mobile_code.send_keys(verfiy_code_input)

        # '登陆'
        self.browser.find_element_by_id('sso_submit').click()
        time.sleep(5)

        cookies = self.browser.get_cookies()
        with open('cookies.json', 'w') as f:
            f.write(json.dumps(cookies))
            self.cookies = json.loads(f)
        print(cookies, "登陆成功")
    def close(self):
        self.browser.close()


class YangShiPin:
    def __init__(self):
        self.cookies = None
        self.chrome_options = Options()
        # self.chrome_options.add_argument('--headless')
        # self.chrome_options.add_argument('--disable-gpu')
        self.browser = webdriver.Chrome('C:\Program Files (x86)\Google\Chrome\Application\chromedriver.exe', options=self.chrome_options)
    def set_cookies(self):
        with open('cookies_y.json', 'r') as f:
            self.cookies = json.loads(f.read())
        for cookie in self.cookies:
            self.browser.add_cookie(cookie)

    def create_session(self):
        self.browser.get("https://mp.yangshipin.cn")
        if self.cookies is None:
            self.set_cookies()
        time.sleep(1)
        self.browser.get("https://mp.yangshipin.cn/management/myContent")

    def login(self):
        self.browser.get(
            "https://mp.yangshipin.cn")
        self.browser.implicitly_wait(5)
        # self.browser.find_element_by_css_selector('#loginPage > div > section > ul > li:nth-child(2) > a').click
        self.browser.find_element_by_xpath(
            '//*[@id="loginPage"]/div/section/ul/li[2]/a').click()
        # self.browser.implicitly_wait(5)
        # self.browser.switch_to.window(self.browser.window_handles[1])
        # # self.browser.find_element_by_xpath('/html/body/div[1]/div[9]/a[1]').click()
        # self.browser.find_element_by_css_selector('#switcher_plogin').click()
        # 账号密码
        # user = self.browser.find_element_by_css_selector('#u')
        # user.send_keys('3414565439')
        # passwd = self.browser.find_element_by_css_selector('#p')
        # passwd.send_keys('Hqhyhlvxj123')

        # # 获取验证码
        # self.browser.find_element_by_css_selector('#login_button').click()
        time.sleep(20)

        cookies = self.browser.get_cookies()
        with open('cookies_y.json', 'w') as f:
            f.write(json.dumps(cookies))
            self.cookies = cookies
        print(cookies, "登陆成功")
    def close(self):
        self.browser.close()

if __name__ == "__main__":
    # tou_tiao = TouTiao()
    # # tou_tiao.login()
    # # calculat_dist()
    # tou_tiao.create_session()

    vsp = YangShiPin()
    # vsp.login()
    vsp.create_session()

