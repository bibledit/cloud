<h2>System configuration</h2>
<p>
How to configure the Bibledit Cloud server at the system level.
This article describes several configuration files.
All of those files reside in the "config" folder of the Bibledit Cloud installation.
The system administrator does this configuration.
</p>

<h3>Proxy server</h3>
<p>
When Bibledit Cloud runs behind a proxy server,
Bibledit needs to be configured to know its own web address that faces the user via that proxy.
The user-facing web address can be set through a configuration file.
The name of the configuration file is "userfacingurl.conf".
It contains the web address, for example: "https://bibledit.org:1500/" without the quotes.
An example configuration file is included in file "userfacingurl.txt".
</p>

<h3>OpenLDAP</h3>
<p>
It is possible to enable an external OpenLDAP server for user authentication,
the file "ldap.txt" contains an example configuration file.
Rename it to "ldap.conf" and make the appropriate settings there.
Note on Bibledit clients: they don't query the LDAP server, they use accounts in Bibledit Cloud instead.
</p>

<h3>Secure web server</h3>
<p>
Bibledit Cloud can be configured to run a secure web server.
This https server needs a private key and public certificates.
The example below uses certificates issued by Let's Encrypt.
It uses the Certbot for obtaining and maintaining the certificates.
The example also assumes that Bibledit Cloud is installed on an Ubuntu VPS,
and that the Apache web server is also installed on that VPS,
but is not needed and can be disabled.
If your situation is different,
<a href="https://certbot.eff.org/docs/" ##external##>the Certbot documentation</a> will provide more information for your specific needs.
</p>
<br>
<p>
Step 1: Stop and disable the Apache web server.
To do this, execute the following two commands as user root:
</p>
<pre>
  $ systemctl stop apache2
  $ systemctl disable apache2
</pre>
<p>
Step 2: Obtain the certificates.
To do this, run the following command as user root:
</p>
<pre>
  $ certbot certonly
</pre>
<p>
The Certbot asks how you would like to authenticate with the ACME CA.
It offers three choices.
Choose the option to spin up a temporary webserver (standalone).
Follow the instructions on the screen to request the initial certificates.
If successful, the certificates will be stored in folder "/etc/letsencrypt/live/yoursite.org".
If not successful, you might have to open port 80 on your VPS for inbound connections.
This could be done with this command:
<pre>
  $ ufw allow 80/tcp
</pre>
<br>
<p>
Step 3: Copy the certificates to Bibledit Cloud.
This example assumes that Bibledit Cloud was installed on Ubuntu from the Bibledit repository.
To copy the obtained certificates into the correct place in Bibledit Cloud,
run the following command as user root:
</p>
<pre>
  $ cp /etc/letsencrypt/live/yoursite.org/*.pem /var/bibledit/bibledit-cloud/config
</pre>
<p>
Where it says "yoursite.org" in the above command,
instead of that enter the name of your own domain where Bibledit Cloud runs.
The configuration path in this example is "/var/bibledit/bibledit-cloud/config".
The configuration path for your Bibledit Cloud instance may be different.
</p>
<br>
<p>
Step 4: Make the certificates readable for Bibledit Cloud.
The certificates were copied as user root, and Bibledit Cloud runs as user bibledit,
so the certificates are not automatically readable by Bibledit.
The three certificate files are:
The server key in file "privkey.pem".
The server certificate in file "cert.pem".
The chain of trusted certificate authorities in file "chain.pem".
To set the permissions correct, run the following command as user root:
</p>
<pre>
  $ chmod ugo+r /var/bibledit/bibledit-cloud/config/*.pem
</pre>
<p>
There is a script in the above config directory.
It assists with steps 3 and 4 above.
This script is "certificates.sh".
Run it as the root user to copy the certificates to Bibledit.
</p>
<p>
Step 5: Restart Bibledit Cloud.
After the certificates have been copied into place,
restart Bibledit Cloud.
It will then load the new certificates.
To restart Bibledit Cloud, run the following command as user root:
</p>
<pre>
  $ systemctl restart bibledit-cloud
</pre>
<p>
Step 6: Verify that https works.
If, for instance, your Bibledit Cloud is accessible at e.g. http://bibledit.org:8090,
then the secure version will be accessible at https://bibledit.org:8091.
Notice that the port number is increased by one.
Open your Bibledit Cloud in a browser and check that the secure connection works.
Double check via command "systemctl status bibledit-cloud" that it listens on http://localhost:... and also on https://localhost...
If it does not listen on https, then the secure web server does not run due to a problem in the certificates.
The Journal within Bibledit Cloud will provide additional diagnostics in this case.
</p>
<br>
<p>
Repeating regular step: Renew the certificates.
The certificates will expire after a couple of months.
So every few month or so renew the certificates.
To do this, run the following command as user root:
</p>
<pre>
  $ certbot renew
</pre>
<p>
After that, follow the steps outlined above again:
3: Copy the certificates to Bibledit Cloud.
4: Make the certificates readable for Bibledit Cloud.
5: Restart Bibledit Cloud.
6: Verify that https works.
</p>
<br>
<p>
To enforce secure http for browser communications,
put a file named "browser.https" in the "config" folder.
The file can be left empty.
Once Bibledit Cloud finds this file,
it will redirect any incoming http browser traffic to https traffic.
Restart Bibledit Cloud for this to take effect.
</p>
<br>
<p>
A file named "client.https" will enforce https for Bibledit client traffic.
It won't redirect incoming http client connections to https though.
It will just generate an error on the client connection.
The user should manually upgrade the client connection.
</p>

<h3>Web server port numbers</h3>
<p>
The default network port number for the plain http server is port number 8080.
To change the port number to another one,
put a different port number in file "network-port" in the "config" folder.
</p>
<br>
<p>
The default network port number for the secure https server is port number 8081.
This port number is the plain http port number plus one.
To change the port number to another one,
put a different port number in file "network-port-secure" in the "config" folder.
If the file contains "0", Bibledit Cloud won't run its secure https server.
</p>

<h3>Administrator details</h3>
<p>
To preset the administrator's account,
put the following three files in the "config" folder.
The files "admin-username" and "admin-password" and "admin-email".
Store one detail in each file.
Then remove file "installed-interface-version" from folder "databases/config/general".
Lastly restart Bibledit-Cloud.
Now the admin credentials will take effect, and you can login using those.
</p>

<h3>Account creation</h3>
<p>
Normally anyone can create an account on Bibledit-Cloud.
If this is not desired, then put a file called "create-no-accounts" in the "config" folder.
Now creation of accounts is no longer possible, and public feedback is no longer possible.
</p>

<h3>Demo</h3>
<p>
If the file "demo" is in the "config" folder,
then Bibledit Cloud behaves as a demo installation.
</p>

<h3>Obfuscating the user interface</h3>
<p>
The user interface can be obfuscated.
See folder "obfuscate" in the source code for more information.
</p>

<h3>When changes take effect</h3>
<p>
After making a change in the files in the "config" folder,
restart Bibledit Cloud, for the new configuration to take effect.
</p>

<h3>Logging all network connections</h3>
<p>
Bibledit Cloud can log all network connections.
To enable this, put a file "log-network" in the "config" folder.
The file can be left empty.
If this file is there,
Bibledit Cloud will log information to file "tmp/log-network.csv".
</p>

<script type="text/javascript" src="../assets/external.js"></script>
