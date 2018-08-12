layui.define(['jquery', 'element', 'form', 'laydate', 'table', 'layer', 'laypage'], function(exports) {
    
    initSearch();
    initTable();
    openLayerBind('add', '添加文章', 'article-add.html', true);
    tableToolbarBind('articles', 'read', function() {
    });

    exports('articles', {});
});

function initSearch() {
    var laydate = layui.laydate;
    laydate.render({
        elem: '#starttime'
    }); 
    laydate.render({
        elem: '#endtime'
    }); 
}

function initTable() {
    renderTable('articles', [[
       {checkbox: true},
       {field: 'title', width: 300, title: '标题', edit: 'text'},
       {field: 'key_word', width: 300, title: '关键字', edit: 'text'},
       {field: 'class', width: 150, title: '分类', edit: 'text'},
       {field: 'label', width: 150, title: '标签', edit: 'text'},
       {field: 'author', width: 100, title: '作者', edit: 'text'},
       {field: 'source', width: 200, title: '来源', edit: 'text'},
       {field: 'status', width: 80, title: '状态'},
       {field: 'time', width: 100, title: '发布时间'},
       {fixed: 'right', width: 170, title: '操作', toolbar: '#articles_toolbar'},
    ]]);
}

