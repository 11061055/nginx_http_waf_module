## BackGround

```
Nginx Web Application Firewall (WAF) Module
```

## Syntax

```
             Syntax:	waf_target uri | off;
             Default:	waf_target off;
             Context:	http, server, location
```

## Conf

```
http {

  server {

    waf_target /waf_target_A;

    location / {
        ......
    }
    
    ######### A waf cluster
    location = /waf_target_A {
        proxy_pass http://A.A.A.A$request_uri;
    }
  }
}
```
```
all requests will first be targeted to waf cluster

if waf return 200, a request continues,

or 500 will be returned to the client.
```

## Wiki

[Wiki](https://github.com/11061055/nginx_http_waf_module/wiki)
