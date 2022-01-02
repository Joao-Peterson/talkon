create table nodes(
    id integer primary key autoincrement, 
    uuid char(36) unique not null,
    addr char(16) not null,
    port integer not null,
    name text not null,
    pic char(45)
);

create table chat(
    id integer primary key autoincrement,
    uuid char(36) unique not null, 
    timestamp datetime not null, 
    msg text not null, 
    foreign key(uuid) references nodes(uuid)
);
