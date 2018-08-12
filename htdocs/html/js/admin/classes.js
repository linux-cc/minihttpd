layui.define(['jquery', 'element', 'form', 'table', 'layer'], function(exports) {

    initTable();
    openLayerBind('add', '添加分类', 'class-add.html');
    tableToolbarBind('classes', 'add', function() {
        openLayer('添加分类', 'class-add.html');
    });

    exports('classes', {});
});

function initTable() {
    renderTable('classes', [[
        {checkbox: true},
        {field: 'name', width: 200, title: '名称'},
        {field: 'father', width: 400, title: '父类'},
        {field: 'articles', width: 80, title: '文章数'},
        {fixed: 'right', width: 215, title: '操作', toolbar: '#classes_toolbar'},
    ]]);
}
