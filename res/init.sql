create table nodes(
    id int not null, 
    name text not null
);

create table chat(
    id int primary key not null,
    node int, 
    timestamp datetime not null, 
    msg text not null, 
    foreign key(node) references nodes(id)
);
