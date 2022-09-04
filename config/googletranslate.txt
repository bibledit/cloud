Configuring Bibledit to use Google Translate
============================================

Google Translate can help you translating your resources into another language.
That gives the translators easier access to those resources in a language they can understand.
To start using translated resources, follow these steps:

Step 1: Obtain a key from Google to access the Google Translate API
-------------------------------------------------------------------

You need to get a JSON key from Google to be able to use the Google Translation service.
Here are the steps to take to get that JSON key from Google.

1.
Choose which Google account to use.
Or create a new Google account and use that.

2.
Open the following page with information about how to setup Google Translate.
https://cloud.google.com/translate/docs/setup
This is a guide to setting up Google Cloud Translation.
The guide helps you with the next steps to take.

You will use the basic edition of Cloud Translation.
The Google Guide assumes that basic adition.

3.
Go to the Google Cloud console.
Go to the project selector page.
Select or create a Google Cloud project.
See the Google Guide for full details.

4.
Enable billing for your Cloud project.
See the Google Guide for full details.

5.
Enable the Cloud Translation API.
To do that, click on the following button in the guide labeled "Enable the API".
Follow the instructions on the screen to complete the process of enabling the API.
See the Google Guide for full details.

6.
Create a service account and keys.
To do that, click on the button in the guide labeled "Go to Create service account".
Follow all the steps in that guide.
At the end you will have a JSON key.
Normally the JSON key will have downloaded to your Downloads folder.
Store that key safely.
See the Google Guide for full details.

7.
Give Bibledit Cloud access to the JSON key.
Store the key in the Bibledit "config" folder as file "googletranslate.json".

Step 2: Install gcloud on the server hosting Bibledit Cloud
-----------------------------------------------------------

Bibledit Cloud authenticates access to Google Translate through the gcloud CLI.
Therefore gcloud needs to be installed on the server.
Information: https://cloud.google.com/sdk/gcloud/

To install gcloud on the Bibledit server, follow these steps:
https://cloud.google.com/sdk/docs/install

If you are installing gcloud on Ubuntu, then the following steps will do it:
$ sudo apt-get install apt-transport-https ca-certificates gnupg
$ echo "deb [signed-by=/usr/share/keyrings/cloud.google.gpg] https://packages.cloud.google.com/apt cloud-sdk main" | sudo tee -a /etc/apt/sources.list.d/google-cloud-sdk.list
$ curl https://packages.cloud.google.com/apt/doc/apt-key.gpg | sudo apt-key --keyring /usr/share/keyrings/cloud.google.gpg add -
$ sudo apt update
$ sudo apt install google-cloud-cli
