-- insert chat message query
insert into chat (uuid, timestamp, msg) values ("uuid", datetime("now", "localtime"), "message");

-- select chat message by uuid and date 
select timestamp, msg from chat where uuid = "123" and timestamp >= "2021-12-24 01:19:29";