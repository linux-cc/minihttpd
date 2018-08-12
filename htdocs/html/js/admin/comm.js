function renderTable(id, cols, url) {
    layui.table.render({
        elem: '#' + id,
        id: id,
        url: url,
        height: 'auto',
        even: true,
        cols: cols,
        done: function(res, curr, count) {
            renderPages(id, cols, url, curr, count);
        }
    });
}

function renderPages(id, cols, url, curr, count) {
    layui.laypage.render({
        elem: id + '_pages',
        layout: ['prev', 'page', 'next', 'skip', 'count'],
        curr: curr,
        count: count,
        limit: 10,
        jump: function(obj, first) {
            if (!first) {
                renderTable(id, cols, url)
            }
        }
    });
}

function openLayerBind(id, title, content, isArticleAdd) {
    var $ = layui.jquery;
    $(window).one('resize', function() {
        $('#' + id).click(function() {
            var index = openLayer(title, content);
            if (isArticleAdd) {
                layui.layer.full(index);
            }
        });
    }).resize();
}

function openLayer(title, content) {
    var index = layui.layer.open({
        title : [title, 'font-size:18px;'],
        type : 2,
        content : content,
        area: ['600px', '250px'],
        btn: ['提交', '取消'],
        yes: function(index, layero) {
            layer.alert('点击了提交按钮');
            layer.close(index);
        },
        btn2: function(index, layero) {
            layer.close(index);
        }
    });
    return index;
}

function tableToolbarBind(id, special, specialFunc) {
    layui.table.on('tool(' + id + ')', function(obj) {
        var data = obj.data; //获得当前行数据
        var layEvent = obj.event; //获得 lay-event 对应的值
        var tr = obj.tr; //获得当前行 tr 的DOM对象

        if (layEvent === 'del') {
            layer.confirm('确定删除?', function(index) {
                obj.del();
                layer.close(index);
                //todo 向服务端发送删除指令
            });
        } else if (layEvent === 'edit') {
            //同步更新缓存对应的值
            obj.update({
                name: '123',
                title: 'xxx'
            });
        } else if (special && layEvent === special) {
            specialFunc();
        }
    });
}
