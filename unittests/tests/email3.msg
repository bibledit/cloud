Delivered-To: receiver@gmail.com
Received: by 10.129.84.8 with SMTP id i8csp116968ywb;
        Tue, 14 Mar 2017 22:21:02 -0700 (PDT)
X-Received: by 10.223.139.152 with SMTP id o24mr1016398wra.61.1489555261964;
        Tue, 14 Mar 2017 22:21:01 -0700 (PDT)
Return-Path: <bibledit-general+bncBDLZXY6F6QNRBPM6UPDAKGQEJ4P3MZI@googlegroups.com>
Received: from mail-wr0-x23b.google.com (mail-wr0-x23b.google.com. [2a00:1450:400c:c0c::23b])
        by mx.google.com with ESMTPS id t19si441424wrb.3.2017.03.14.22.21.01
        (version=TLS1_2 cipher=ECDHE-RSA-AES128-GCM-SHA256 bits=128/128);
        Tue, 14 Mar 2017 22:21:01 -0700 (PDT)
Received-SPF: pass (google.com: domain of bibledit-general+bncbdlzxy6f6qnrbpm6updakgqej4p3mzi@googlegroups.com designates 2a00:1450:400c:c0c::23b as permitted sender) client-ip=2a00:1450:400c:c0c::23b;
Authentication-Results: mx.google.com;
       dkim=pass header.i=@googlegroups.com;
       spf=pass (google.com: domain of bibledit-general+bncbdlzxy6f6qnrbpm6updakgqej4p3mzi@googlegroups.com designates 2a00:1450:400c:c0c::23b as permitted sender) smtp.mailfrom=bibledit-general+bncBDLZXY6F6QNRBPM6UPDAKGQEJ4P3MZI@googlegroups.com
Received: by mail-wr0-x23b.google.com with SMTP id l37sf1030769wrc.0;
        Tue, 14 Mar 2017 22:21:01 -0700 (PDT)
DKIM-Signature: v=1; a=rsa-sha256; c=relaxed/relaxed;
        d=googlegroups.com; s=20161025;
        h=sender:mime-version:from:date:message-id:subject:to
         :x-original-sender:x-original-authentication-results:reply-to
         :precedence:mailing-list:list-id:x-spam-checked-in-group:list-post
         :list-help:list-archive:list-subscribe:list-unsubscribe;
        bh=JeDF1voi+s0dnAg7JAgsRsHs4gMiAE4Bmv56OlZH2oY=;
        b=OAQFNMphJypdhkgp0+9rUo6tlblHkbgiYdOK0MfiPiadvc7N8hCaVKGuCsn8GK/mo9
         fvYkH1pNP20+W/A/L5sZoZw9hvr0pu4GW2atLKLhBDq+9Be39DIIK9HHe4ITvPkPaIzT
         JSwjsyJ0ri3FOjySt2DB8FwJ6DaxRwpZvsc3AA7CWKyxi+r3C18cAxoDWe9+W3Od58jJ
         cg5AmSiyV6d92r8F0Ed06pt6X7wFVSMSYQqhs4S7E5Nrme4INx+JO40biX3Mf7tnXpfU
         jhxTIYj2uFKhSqCAukRmnItRBcvorFDfe3kyitlaPzNcxlpl9x6NYxOzM5X64/Apd0Ix
         R3PA==
X-Google-DKIM-Signature: v=1; a=rsa-sha256; c=relaxed/relaxed;
        d=1e100.net; s=20161025;
        h=sender:x-gm-message-state:mime-version:from:date:message-id:subject
         :to:x-original-sender:x-original-authentication-results:reply-to
         :precedence:mailing-list:list-id:x-spam-checked-in-group:list-post
         :list-help:list-archive:list-subscribe:list-unsubscribe;
        bh=JeDF1voi+s0dnAg7JAgsRsHs4gMiAE4Bmv56OlZH2oY=;
        b=cO1+0+y059hQNlLuNax2K7f/X3E9mjmvupcOZbDhoBSKsk/oiwkFFQganNvdQNX8gP
         HXEt07hJxr8kDTLH806uBbnKLWIf+lqKKrr6WSimzIyK6523XZciiEx64Sd7UT3FY8dO
         jy94WiBWlaqeLAeF5c8OvlHXhfdARbDUtzH3kUay+2tqBbegGsfCP0EUwSW+Rz3kttT3
         wH0ZN1JoVX7vnUhS1QeMGtMzTtER1M2mU1V2MEwu2+pkDktWmsfkzI48/BAWNMs0GQnj
         IFDYrCg972l83SwA6J4trUPifTT0/j3Ns7vQLL6gLur5ZU/YW8SVN2OhMTiymfUJxV20
         wtZg==
Sender: bibledit-general@googlegroups.com
X-Gm-Message-State: AFeK/H3lSiizfioDeVJ5FKipjnn/jFHaWzJkd82bytLIrPrU8Ho1OoJV3VX2LZVv3BuQ1A==
X-Received: by 10.25.196.130 with SMTP id u124mr5559lff.6.1489555261536;
        Tue, 14 Mar 2017 22:21:01 -0700 (PDT)
X-BeenThere: bibledit-general@googlegroups.com
Received: by 10.46.9.134 with SMTP id 128ls100888ljj.4.gmail; Tue, 14 Mar 2017
 22:21:01 -0700 (PDT)
X-Received: by 10.25.199.66 with SMTP id x63mr118183lff.4.1489555261242;
        Tue, 14 Mar 2017 22:21:01 -0700 (PDT)
Received: from mail-wm0-x22e.google.com (mail-wm0-x22e.google.com. [2a00:1450:400c:c09::22e])
        by gmr-mx.google.com with ESMTPS id y19si519983wmd.0.2017.03.14.22.21.01
        for <bibledit-general@googlegroups.com>
        (version=TLS1_2 cipher=ECDHE-RSA-AES128-GCM-SHA256 bits=128/128);
        Tue, 14 Mar 2017 22:21:01 -0700 (PDT)
Received-SPF: neutral (google.com: 2a00:1450:400c:c09::22e is neither permitted nor denied by best guess record for domain of sender@domain.net) client-ip=2a00:1450:400c:c09::22e;
Received: by mail-wm0-x22e.google.com with SMTP id n11so78857942wma.1
        for <bibledit-general@googlegroups.com>; Tue, 14 Mar 2017 22:21:01 -0700 (PDT)
X-Received: by 10.28.210.133 with SMTP id j127mr2641337wmg.64.1489555260725;
 Tue, 14 Mar 2017 22:21:00 -0700 (PDT)
MIME-Version: 1.0
Received: by 10.223.146.162 with HTTP; Tue, 14 Mar 2017 22:20:57 -0700 (PDT)
Received: by 10.223.146.162 with HTTP; Tue, 14 Mar 2017 22:20:57 -0700 (PDT)
From: Sender <sender@domain.net>
Date: Wed, 15 Mar 2017 14:20:57 +0900
Message-ID: <CAPW6x-xW=HdKErAiTG3=+_nUy0_W49HRoWamcpDRfDbe-Cf+PQ@mail.gmail.com>
Subject: Message encoded with quoted-printable
To: bibledit-general@googlegroups.com
Content-Type: multipart/alternative; boundary=001a11472176307118054abe1cc9
X-Original-Sender: sender@domain.net
X-Original-Authentication-Results: gmr-mx.google.com;       dkim=pass
 header.i=@sender-net.20150623.gappssmtp.com;       spf=neutral
 (google.com: 2a00:1450:400c:c09::22e is neither permitted nor denied by best
 guess record for domain of sender@domain.net) smtp.mailfrom=sender@domain.net
Reply-To: bibledit-general@googlegroups.com
Precedence: list
Mailing-list: list bibledit-general@googlegroups.com; contact bibledit-general+owners@googlegroups.com
List-ID: <bibledit-general.googlegroups.com>
X-Spam-Checked-In-Group: bibledit-general@googlegroups.com
X-Google-Group-Id: 774256976901
List-Post: <https://groups.google.com/group/bibledit-general/post>, <mailto:bibledit-general@googlegroups.com>
List-Help: <https://groups.google.com/support/>, <mailto:bibledit-general+help@googlegroups.com>
List-Archive: <https://groups.google.com/group/bibledit-general
List-Subscribe: <https://groups.google.com/group/bibledit-general/subscribe>, <mailto:bibledit-general+subscribe@googlegroups.com>
List-Unsubscribe: <mailto:googlegroups-manage+774256976901+unsubscribe@googlegroups.com>,
 <https://groups.google.com/group/bibledit-general/subscribe>

--001a11472176307118054abe1cc9
Content-Type: text/plain; charset=UTF-8
Content-Transfer-Encoding: quoted-printable

Receiver,

Can you make it so that=E2=80=94 when in Basic Mode and staying in the same=
 verse,
the Resources will not change positions when one returns to that screen?

=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D
Sender

USA: 111-222-3333
Indonesia: 1234567890
P.O. Box 111, Dry Grounds, AA 111111

http://website.info
http://website.info

Firman Allah adalah Kunci Kehidupan
=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D

--=20
You received this message because you are subscribed to the Google Groups "=
Bibledit-General" group.
To unsubscribe from this group and stop receiving emails from it, send an e=
mail to bibledit-general+unsubscribe@googlegroups.com.
To post to this group, send email to bibledit-general@googlegroups.com.
Visit this group at https://groups.google.com/group/bibledit-general.
For more options, visit https://groups.google.com/d/optout.

--001a11472176307118054abe1cc9
Content-Type: text/html; charset=UTF-8
Content-Transfer-Encoding: quoted-printable

<div dir=3D"auto">Sender,<div dir=3D"auto"><br></div><div dir=3D"auto">Can yo=
u make it so that=E2=80=94 when in Basic Mode and staying in the same verse=
, the Resources will not change positions when one returns to that screen?<=
br><br><div data-smartmail=3D"gmail_signature" dir=3D"auto">=3D=3D=3D=3D=3D=
=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D<br>Sender<br><br><br>=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D=3D<br>=
<br>=C2=A0=C2=A0=C2=A0 </div></div></div>

<p></p>

-- <br />
You received this message because you are subscribed to the Google Groups &=
quot;Bibledit-General&quot; group.<br />
To unsubscribe from this group and stop receiving emails from it, send an e=
mail to <a href=3D"mailto:bibledit-general+unsubscribe@googlegroups.com">bi=
bledit-general+unsubscribe@googlegroups.com</a>.<br />
To post to this group, send email to <a href=3D"mailto:bibledit-general@goo=
glegroups.com">bibledit-general@googlegroups.com</a>.<br />
Visit this group at <a href=3D"https://groups.google.com/group/bibledit-gen=
eral">https://groups.google.com/group/bibledit-general</a>.<br />
For more options, visit <a href=3D"https://groups.google.com/d/optout">http=
s://groups.google.com/d/optout</a>.<br />

--001a11472176307118054abe1cc9--
