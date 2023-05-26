
// StudentInfoClientDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "StudentInfoClient.h"
#include "StudentInfoClientDlg.h"
#include "afxdialogex.h"
#include <format>
#include <string>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using enum QUDP_TYPE;
using namespace std;

// CStudentInfoClientDlg 对话框
CStudentInfoClientDlg::CStudentInfoClientDlg(CWnd* pParent /*=nullptr*/)
  : CDialogEx(IDD_STUDENTINFOCLIENT_DIALOG, pParent)
  , pFile(nullptr)
  , m_editTest(_T(""))
  , m_szClassId(_T(""))
  , m_szClassName(_T(""))
  , m_szCourseId(_T(""))
  , m_szCourseName(_T(""))
  , m_szScoreCourseId(_T(""))
  , m_szScoreStuId(_T(""))
  , m_szScoreNum(_T(""))
  , m_szStuClassId(_T(""))
  , m_szStuId(_T(""))
  , m_szStuName(_T(""))
  , m_checkClassId(FALSE)
  , m_checkClassName(FALSE)
  , m_checkCourseId(FALSE)
  , m_checkCourseName(FALSE)
  , m_checkStuId(FALSE)
  , m_checkStuName(FALSE)
  , m_checkCourseNum(FALSE)
{
  m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CStudentInfoClientDlg::~CStudentInfoClientDlg()
{
  m_qudp.StopWork();
  Sleep(500);
  if (pFile) {
    fclose(pFile);
  }
}

void CStudentInfoClientDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialogEx::DoDataExchange(pDX);
  DDX_Control(pDX, ID_TREE_DB, m_treeDB);
  DDX_Control(pDX, ID_LIST_DATA, m_listCtrlData);
  DDX_Text(pDX, IDC_EDIT1, m_editTest);
  DDX_Text(pDX, CLASS_ID, m_szClassId);
  DDX_Text(pDX, CLASS_NAME, m_szClassName);
  DDX_Text(pDX, COURSE_ID, m_szCourseId);
  DDX_Text(pDX, COURSE_NAME, m_szCourseName);
  DDX_Text(pDX, SCORE_COURSE_ID, m_szScoreCourseId);
  DDX_Text(pDX, SCORE_STUDENT_ID, m_szScoreStuId);
  DDX_Text(pDX, SCORE_NUM, m_szScoreNum);
  DDX_Text(pDX, STU_CLASS_ID, m_szStuClassId);
  DDX_Text(pDX, STU_ID, m_szStuId);
  DDX_Text(pDX, STU_NAME, m_szStuName);
  DDX_Check(pDX, CHECK_CLASS_ID, m_checkClassId);
  DDX_Check(pDX, CHECK_CLASS_NAME, m_checkClassName);
  DDX_Check(pDX, CHECK_COURSE_ID, m_checkCourseId);
  DDX_Check(pDX, CHECK_COURSE_NAME, m_checkCourseName);
  DDX_Check(pDX, CHECK_STU_ID, m_checkStuId);
  DDX_Check(pDX, CHECK_STU_NAME, m_checkStuName);
  DDX_Check(pDX, CHECK_COURSE_NUM, m_checkCourseNum);
}

BEGIN_MESSAGE_MAP(CStudentInfoClientDlg, CDialogEx)
  ON_WM_PAINT()
  ON_WM_QUERYDRAGICON()
  ON_BN_CLICKED(IDOK, &CStudentInfoClientDlg::OnBnClickedOk)
  ON_NOTIFY(NM_DBLCLK, ID_TREE_DB, &CStudentInfoClientDlg::OnDblclkTreeDb)
  ON_NOTIFY(NM_DBLCLK, ID_LIST_DATA, &CStudentInfoClientDlg::OnDblclkListData)
  ON_BN_CLICKED(CLASS_INSERT, &CStudentInfoClientDlg::OnBnClickedClassInsert)
  ON_BN_CLICKED(CLASS_DELETE, &CStudentInfoClientDlg::OnBnClickedClassDelete)
  ON_BN_CLICKED(CLASS_UPDATE, &CStudentInfoClientDlg::OnBnClickedClassUpdate)
  ON_BN_CLICKED(SELECT, &CStudentInfoClientDlg::OnBnClickedClassSelect)
  ON_BN_CLICKED(COURSE_INSERT, &CStudentInfoClientDlg::OnBnClickedCourseInsert)
  ON_BN_CLICKED(COURSE_DELETE, &CStudentInfoClientDlg::OnBnClickedCourseDelete)
  ON_BN_CLICKED(COURSE_UPDATE, &CStudentInfoClientDlg::OnBnClickedCourseUpdate)
  ON_BN_CLICKED(COURSE_SELECT, &CStudentInfoClientDlg::OnBnClickedCourseSelect)
  ON_BN_CLICKED(SCORE_INSERT, &CStudentInfoClientDlg::OnBnClickedScoreInsert)
  ON_BN_CLICKED(SCORE_DELETE, &CStudentInfoClientDlg::OnBnClickedScoreDelete)
  ON_BN_CLICKED(SCORE_UPDATE, &CStudentInfoClientDlg::OnBnClickedScoreUpdate)
  ON_BN_CLICKED(SCORE_SELECT, &CStudentInfoClientDlg::OnBnClickedScoreSelect)
  ON_BN_CLICKED(STU_INSERT, &CStudentInfoClientDlg::OnBnClickedStuInsert)
  ON_BN_CLICKED(STU_DELETE, &CStudentInfoClientDlg::OnBnClickedStuDelete)
  ON_BN_CLICKED(STU_UPDATE, &CStudentInfoClientDlg::OnBnClickedStuUpdate)
  ON_BN_CLICKED(STU_SELECT, &CStudentInfoClientDlg::OnBnClickedStuSelect)
  ON_BN_CLICKED(ID_QUERY, &CStudentInfoClientDlg::OnBnClickedQuery)
  ON_COMMAND(MN_DELETE, &CStudentInfoClientDlg::OnDelete)
  ON_NOTIFY(NM_RCLICK, ID_LIST_DATA, &CStudentInfoClientDlg::OnRclickListData)
END_MESSAGE_MAP()


// CStudentInfoClientDlg 消息处理程序

BOOL CStudentInfoClientDlg::OnInitDialog()
{
  CDialogEx::OnInitDialog();

  // 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
  //  执行此操作
  SetIcon(m_hIcon, TRUE);			// 设置大图标
  SetIcon(m_hIcon, FALSE);		// 设置小图标

  m_listCtrlData.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
  m_listCtrlData.SetItemCount(INT_MAX);

  m_treeDB.InsertItem("localhost");
  m_treeDB.Expand(m_treeDB.GetRootItem(), TVE_EXPAND);

  // 将std::function对象绑定到成员函数
  m_updateUIFunc = std::bind(&CStudentInfoClientDlg::UpdateUI,
    this, std::placeholders::_1, std::placeholders::_2);
  //192.168.157.11
  if (m_qudp.ConnectServer(9527, "127.0.0.1")) {
    m_qudp.StartWork(m_updateUIFunc);
    //发送握手包，用于确认会话ID
    m_qudp.SendPackage(Package(QUDP_SYN, m_qudp.m_sockChannel.m_nConvId, 0));
  }

  DBOperatePacket dbPacket;
  std::memcpy(dbPacket.m_szDbName, "show db", sizeof("show db"));
  Package packet(C2S_SHOW_DB, m_qudp.m_sockChannel.m_nConvId,
    0, (char*)&dbPacket, sizeof(dbPacket));
  m_qudp.SendPackage(packet);

  m_editTest = "25";
  UpdateData(false);
  return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CStudentInfoClientDlg::OnPaint()
{
  if (IsIconic())
  {
    CPaintDC dc(this); // 用于绘制的设备上下文

    SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

    // 使图标在工作区矩形中居中
    int cxIcon = GetSystemMetrics(SM_CXICON);
    int cyIcon = GetSystemMetrics(SM_CYICON);
    CRect rect;
    GetClientRect(&rect);
    int x = (rect.Width() - cxIcon + 1) / 2;
    int y = (rect.Height() - cyIcon + 1) / 2;

    // 绘制图标
    dc.DrawIcon(x, y, m_hIcon);
  }
  else
  {
    CDialogEx::OnPaint();
  }
}

//当用户拖动最小化窗口时系统调用此函数取得光标
HCURSOR CStudentInfoClientDlg::OnQueryDragIcon()
{
  return static_cast<HCURSOR>(m_hIcon);
}



void CStudentInfoClientDlg::OnBnClickedOk()
{

  return;
  //获取所有表
  DBOperatePacket dbPacket;
  std::memcpy(dbPacket.m_szDbName, "mysql", sizeof("mysql"));
  Package packet(C2S_SHOW_TABLE, m_qudp.m_sockChannel.m_nConvId,
    m_qudp.m_sockChannel.m_nNextSendSeq++, (char*)&dbPacket, sizeof(dbPacket));

  m_qudp.SendPackage(packet);
  return;

#if 0

  DBOperatePacket dbPacket;
  std::memcpy(dbPacket.m_szDbName, "show db", sizeof("show db"));
  Package packet(C2S_SHOW_DB, m_qudp.m_sockChannel.m_nConvId,
    0, (char*)&dbPacket, sizeof(dbPacket));
  m_qudp.SendPackage(packet);
  return;

  UpdateData(true);
  if (!pFile) {
    auto str = m_editTest.GetBuffer();
    pFile = fopen(str, "rb+");
    if (pFile) {
      fseek(pFile, 0, SEEK_END);
      fgetpos(pFile, &nFileSize);
      fseek(pFile, 0, SEEK_SET);
    }
  }
  if (nFileSize != 0) {
    m_qudp.Send((char*)&nFileSize, sizeof(nFileSize));
  }

#endif // 0

}

void CStudentInfoClientDlg::UpdateUI2(QUDP_TYPE type, size_t nLen, std::vector<char>& dataBuf)
{
  switch (type) {
    case S2C_SHOW_DB: {
      //DBPacket dbPacket = *((DBPacket*)packetBuf);
      //HTREEITEM root = m_treeDB.GetRootItem();
      //m_treeDB.InsertItem(dbPacket.m_data, root);
      //m_treeDB.Expand(root, TVE_EXPAND);
      break;
    }
    case S2C_SHOW_TABLE: {
      auto hItem = m_treeDB.GetSelectedItem();
      //int nHeader = sizeof(Package) - QUDP_MSS;
      //auto start = dataBuf.begin() + nHeader;

      //TablePacket dbPacket = *((TablePacket*)packetBuf);
      //m_treeDB.InsertItem(dbPacket.m_data, hItem);
      //m_treeDB.Expand(hItem, TVE_EXPAND);


      break;
    }
  }
}


void CStudentInfoClientDlg::UpdateUI(QUDP_TYPE type, char* packetBuf)
{
  switch (type) {
    case S2C_INSERT: {
      auto packet = *(m_qudp.m_sockChannel.m_mapRecv.begin());
      string msg;
      msg.assign(packet.second.m_buf, packet.second.m_nSize);
      if (msg == string("成功")) {
        ClearListCtrl();
        std::string sql = std::format("select * from {};", m_currentTable.GetBuffer());
        Package packet(C2S_SELECT, m_qudp.m_sockChannel.m_nConvId,
          m_qudp.m_sockChannel.m_nNextSendSeq++, (char*)sql.c_str(), sql.size() + 1);
        m_qudp.SendPackage(packet);
      }
      else {
        AfxMessageBox(msg.c_str());
      }

      m_qudp.m_sockChannel.m_mapRecv.erase(packet.second.m_nSeq);
      break;
    }
    case S2C_SELECT: {
      auto packet = *(m_qudp.m_sockChannel.m_mapRecv.begin());
      std::string str;
      str.assign(packet.second.m_buf, packet.second.m_nSize);
      m_nCount++;
      int index = 0;
      const std::string split = "----";
      std::string split2 = "::";


      size_t nCount = 0; //共有多少列字段
      size_t pos = 0;
      while ((pos = str.find(split, pos)) != std::string::npos) {
        ++nCount;
        pos += split.length();
      }

      std::vector<std::string> vctValue;
      int nColumn = 0;
      while (!str.empty()) {
        index = str.find(split);
        string sub = str.substr(0, index);
        str = str.substr(index + split.length());

        int index2 = sub.find(split2);
        string key = sub.substr(0, index2);
        string value = sub.substr(index2 + split2.length());
        LOG(Log("%s : %s\n", key.c_str(), value.c_str()));
        vctValue.push_back(value);

        int nColumnCount = m_listCtrlData.GetHeaderCtrl()->GetItemCount();
        if (nColumnCount < nCount) {
          m_listCtrlData.InsertColumn(nColumn, key.c_str());
          m_listCtrlData.SetColumnWidth(nColumn++, 130);
          Sleep(10);
        }

        index++;
      }
      // 将key插入到第一列
      int row = m_listCtrlData.GetItemCount();
      m_listCtrlData.InsertItem(row, vctValue[0].c_str());
      for (int i = 1; i < vctValue.size(); ++i) {
        m_listCtrlData.SetItemText(row, i, vctValue[i].c_str());
      }
      m_qudp.m_sockChannel.m_mapRecv.erase(packet.second.m_nSeq);
      break;
    }
    case S2C_SHOW_DB: {

      auto packet = *(m_qudp.m_sockChannel.m_mapRecv.begin());
      DBPacket dbPacket = *((DBPacket*)packet.second.m_buf);
      if (string(dbPacket.m_data) == string("student_info")) {
        HTREEITEM root = m_treeDB.GetRootItem();
        m_treeDB.InsertItem(dbPacket.m_data, root);
        m_vctDb.push_back(std::string(dbPacket.m_data));
        m_treeDB.Expand(root, TVE_EXPAND);
      }


      m_qudp.m_sockChannel.m_mapRecv.erase(packet.second.m_nSeq);

      break;
    }
    case S2C_SHOW_TABLE: {
      auto packet = *(m_qudp.m_sockChannel.m_mapRecv.begin());
      TablePacket tablePacket = *((TablePacket*)packet.second.m_buf);

      auto hItem = m_treeDB.GetSelectedItem();
      m_treeDB.InsertItem(tablePacket.m_data, hItem);
      m_treeDB.Expand(hItem, TVE_EXPAND);

      m_qudp.m_sockChannel.m_mapRecv.erase(packet.second.m_nSeq);
      break;
    }
  }
}


void CStudentInfoClientDlg::OnDblclkTreeDb(NMHDR* pNMHDR, LRESULT* pResult)
{
  auto hItem = m_treeDB.GetSelectedItem();

  CString itemName = m_treeDB.GetItemText(hItem);
  if (!itemName.IsEmpty()) {

    if (!m_treeDB.ItemHasChildren(hItem)) { //没有子节点才获取表名
      //判断是不是表名
      std::string parentName = std::string(m_treeDB.GetItemText(m_treeDB.GetParentItem(hItem)).GetBuffer());
      if (std::find(m_vctDb.begin(), m_vctDb.end(), parentName) != m_vctDb.end()) {
        //父节点是数据库名，则请求该表数据，itemName 就是双击的表名
        //DBOperatePacket opPacket;
        //std::memcpy(opPacket.m_szTableName, itemName.GetBuffer(), itemName.GetLength() + 1);

        //清空列表数据
        ClearListCtrl();
        m_currentTable = itemName; //赋值给当前表名

        std::string sql = std::format("select * from {};", itemName.GetBuffer());
        Package packet(C2S_SELECT, m_qudp.m_sockChannel.m_nConvId,
          m_qudp.m_sockChannel.m_nNextSendSeq++, (char*)sql.c_str(), sql.size() + 1);
        m_qudp.SendPackage(packet);
      }
      else { //否则获取该数据库的所有表
        m_currentDb = itemName; //赋值给当前数据库名

        DBOperatePacket dbPacket;
        std::memcpy(dbPacket.m_szDbName, itemName.GetBuffer(), itemName.GetLength() + 1);
        Package packet(C2S_SHOW_TABLE, m_qudp.m_sockChannel.m_nConvId,
          m_qudp.m_sockChannel.m_nNextSendSeq++, (char*)&dbPacket, sizeof(dbPacket));
        m_qudp.SendPackage(packet);
      }

    }


  }


  *pResult = 0;
}


void CStudentInfoClientDlg::OnDblclkListData(NMHDR* pNMHDR, LRESULT* pResult)
{
  UpdateData(true);
  LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

  int nItem = pNMItemActivate->iItem;
  int nSubItem = pNMItemActivate->iSubItem;
  m_nSelectItem = nItem;
  if (m_currentTable == "class") {
    m_szClassId = m_listCtrlData.GetItemText(nItem, 1);
    m_szClassName = m_listCtrlData.GetItemText(nItem, 2);
  }
  if (m_currentTable == "course") {
    m_szCourseId = m_listCtrlData.GetItemText(nItem, 1);
    m_szCourseName = m_listCtrlData.GetItemText(nItem, 2);
  }
  if (m_currentTable == "score") {
    m_szScoreStuId = m_listCtrlData.GetItemText(nItem, 0);
    m_szScoreCourseId = m_listCtrlData.GetItemText(nItem, 1);
    m_szScoreNum = m_listCtrlData.GetItemText(nItem, 2);
  }
  if (m_currentTable == "student") {
    
    //m_nStudentId = atoi(m_listCtrlData.GetItemText(nItem, 0));
    m_szStuId = m_listCtrlData.GetItemText(nItem, 1);
    m_szStuName = m_listCtrlData.GetItemText(nItem, 2);
    m_szStuClassId = m_listCtrlData.GetItemText(nItem, 3);
  }

  UpdateData(false);
  *pResult = 0;
}

/*
班级表的增删改查
*/
void CStudentInfoClientDlg::OnBnClickedClassInsert()
{
  UpdateData(true);
  if (!m_szClassId.IsEmpty() && !m_szClassName.IsEmpty()) {
    std::string sql = std::format("insert into class (class_id, class_name) \
        values ({}, '{}');", m_szClassId.GetBuffer(), m_szClassName.GetBuffer());
    Package packet(C2S_INSERT, m_qudp.m_sockChannel.m_nConvId,
      m_qudp.m_sockChannel.m_nNextSendSeq++, (char*)sql.c_str(), sql.size() + 1);
    m_qudp.SendPackage(packet);
  }
  else {
    AfxMessageBox("字段不能为空");
  }

}


void CStudentInfoClientDlg::OnBnClickedClassDelete()
{
  UpdateData(true);
  if (!m_szClassId.IsEmpty()) {
    std::string sql = std::format("delete from class where class_id={}", m_szClassId.GetBuffer());
    Package packet(C2S_DELETE, m_qudp.m_sockChannel.m_nConvId,
      m_qudp.m_sockChannel.m_nNextSendSeq++, (char*)sql.c_str(), sql.size() + 1);
    m_qudp.SendPackage(packet);
  }
  else {
    AfxMessageBox("字段不能为空");
  }
}


void CStudentInfoClientDlg::OnBnClickedClassUpdate()
{
  UpdateData(true);
  if (!m_szClassId.IsEmpty() && !m_szClassName.IsEmpty()) {
    std::string sql = std::format("update class set class_name='{}' where class_id={};",
      m_szClassName.GetBuffer(), m_szClassId.GetBuffer());
    Package packet(C2S_UPDATE, m_qudp.m_sockChannel.m_nConvId,
      m_qudp.m_sockChannel.m_nNextSendSeq++, (char*)sql.c_str(), sql.size() + 1);
    m_qudp.SendPackage(packet);
  }
  else {
    AfxMessageBox("字段不能为空");
  }
}


void CStudentInfoClientDlg::OnBnClickedClassSelect()
{
  UpdateData(true);
  std::string sql = "select * from class where 1=1 ";
  if (!m_szClassName.IsEmpty()) {
    sql += std::format(" and class_name='{}' ", m_szClassName.GetBuffer());
  }
  if (!m_szClassId.IsEmpty()) {
    sql += std::format(" and class_id={};", m_szClassId.GetBuffer());
  }
  Package packet(C2S_SELECT, m_qudp.m_sockChannel.m_nConvId,
    m_qudp.m_sockChannel.m_nNextSendSeq++, (char*)sql.c_str(), sql.size() + 1);
  ClearListCtrl();
  m_qudp.SendPackage(packet);
}




/*
课程表的增删改查
*/
void CStudentInfoClientDlg::OnBnClickedCourseInsert()
{
  UpdateData(true);
  if (!m_szCourseId.IsEmpty() && !m_szCourseName.IsEmpty()) {
    std::string sql = std::format("insert into course (course_id, course_name) \
        values ({}, '{}');", m_szCourseId.GetBuffer(), m_szCourseName.GetBuffer());
    Package packet(C2S_INSERT, m_qudp.m_sockChannel.m_nConvId,
      m_qudp.m_sockChannel.m_nNextSendSeq++, (char*)sql.c_str(), sql.size() + 1);
    m_qudp.SendPackage(packet);
  }
  else {
    AfxMessageBox("字段不能为空");
  }
}


void CStudentInfoClientDlg::OnBnClickedCourseDelete()
{
  UpdateData(true);
  if (!m_szCourseId.IsEmpty()) {
    std::string sql = std::format("delete from course where course_id={}", m_szCourseId.GetBuffer());
    Package packet(C2S_DELETE, m_qudp.m_sockChannel.m_nConvId,
      m_qudp.m_sockChannel.m_nNextSendSeq++, (char*)sql.c_str(), sql.size() + 1);
    m_qudp.SendPackage(packet);
  }
  else {
    AfxMessageBox("字段不能为空");
  }
}


void CStudentInfoClientDlg::OnBnClickedCourseUpdate()
{
  UpdateData(true);
  if (!m_szCourseId.IsEmpty()) {
    std::string sql = std::format("update course set course_name='{}' where course_id={};",
      m_szCourseName.GetBuffer(), m_szCourseId.GetBuffer());
    Package packet(C2S_UPDATE, m_qudp.m_sockChannel.m_nConvId,
      m_qudp.m_sockChannel.m_nNextSendSeq++, (char*)sql.c_str(), sql.size() + 1);
    m_qudp.SendPackage(packet);
  }
  else {
    AfxMessageBox("字段不能为空");
  }
}


void CStudentInfoClientDlg::OnBnClickedCourseSelect()
{
  UpdateData(true);
  std::string sql = "select * from course where 1=1 ";
  if (!m_szCourseName.IsEmpty()) {
    sql += std::format(" and course_name='{}' ", m_szCourseName.GetBuffer());
  }
  if (!m_szCourseId.IsEmpty()) {
    sql += std::format(" and course_id={};", m_szCourseId.GetBuffer());
  }
  Package packet(C2S_SELECT, m_qudp.m_sockChannel.m_nConvId,
    m_qudp.m_sockChannel.m_nNextSendSeq++, (char*)sql.c_str(), sql.size() + 1);
  ClearListCtrl();
  m_qudp.SendPackage(packet);
}


/*
成绩表的增删改查
*/
void CStudentInfoClientDlg::OnBnClickedScoreInsert()
{
  UpdateData(true);
  if (!m_szScoreCourseId.IsEmpty() && !m_szScoreStuId.IsEmpty()) {
    std::string sql = std::format("insert into score (course_id, student_id, score) \
        values ({}, {}, {});", m_szScoreCourseId.GetBuffer(), 
      m_szScoreStuId.GetBuffer(),
      m_szScoreNum.GetBuffer());

    Package packet(C2S_INSERT, m_qudp.m_sockChannel.m_nConvId,
      m_qudp.m_sockChannel.m_nNextSendSeq++, (char*)sql.c_str(), sql.size() + 1);
    m_qudp.SendPackage(packet);
  }
  else {
    AfxMessageBox("字段不能为空");
  }
}


void CStudentInfoClientDlg::OnBnClickedScoreDelete()
{
  UpdateData(true);
  if (!m_szScoreStuId.IsEmpty() && !m_szScoreCourseId.IsEmpty()) {
    std::string sql = std::format("delete from score where course_id={} and student_id={};", 
      m_szScoreCourseId.GetBuffer(), m_szScoreStuId.GetBuffer());
    Package packet(C2S_DELETE, m_qudp.m_sockChannel.m_nConvId,
      m_qudp.m_sockChannel.m_nNextSendSeq++, (char*)sql.c_str(), sql.size() + 1);
    m_qudp.SendPackage(packet);
  }
  else {
    AfxMessageBox("字段不能为空");
  }
}


void CStudentInfoClientDlg::OnBnClickedScoreUpdate()
{
  CString szScoreStuId = m_szScoreStuId;
  CString szScoreCourseId = m_szScoreCourseId;
  UpdateData(true);

  std::string sql = "update score set";
  if (!m_szScoreCourseId.IsEmpty()) {
    sql += std::format("  course_id={}, ", m_szScoreCourseId.GetBuffer());
  }
  if (!m_szScoreStuId.IsEmpty()) {
    sql += std::format("  student_id={}, ", m_szScoreStuId.GetBuffer());
  }
  if (!m_szScoreNum.IsEmpty()) {
    sql += std::format("  score={}, ", m_szScoreNum.GetBuffer());
  }
  if (m_szScoreCourseId.IsEmpty() && m_szScoreStuId.IsEmpty() && m_szScoreNum.IsEmpty()) {
    AfxMessageBox("字段不能全为空");
    return;
  }
  int index = sql.rfind(',');
  sql.replace(index, index + 1, " ");
  sql += std::format(" where course_id={} and student_id={};", 
    szScoreCourseId.GetBuffer(), szScoreStuId.GetBuffer());

  Package packet(C2S_UPDATE, m_qudp.m_sockChannel.m_nConvId,
    m_qudp.m_sockChannel.m_nNextSendSeq++, (char*)sql.c_str(), sql.size() + 1);
  m_qudp.SendPackage(packet);


#if 0
  if (!m_szScoreStuId.IsEmpty() && !m_szScoreCourseId.IsEmpty()) {
    std::string sql = std::format("update score set course_id={}, student_id={}, score={}\
         where course_id={} and student_id={};",
      m_szScoreCourseId.GetBuffer(), m_szScoreStuId.GetBuffer(),
      m_szScoreNum.GetBuffer(), szScoreCourseId.GetBuffer(), szScoreStuId.GetBuffer());
    Package packet(C2S_UPDATE, m_qudp.m_sockChannel.m_nConvId,
      m_qudp.m_sockChannel.m_nNextSendSeq++, (char*)sql.c_str(), sql.size() + 1);
    m_qudp.SendPackage(packet);
  }
  else {
    AfxMessageBox("字段不能为空");
  }
#endif // 0

}


void CStudentInfoClientDlg::OnBnClickedScoreSelect()
{
  UpdateData(true);
  std::string sql = "select * from score where 1=1 ";
  if (!m_szScoreCourseId.IsEmpty()) {
    sql += std::format(" and course_id={} ", m_szScoreCourseId.GetBuffer());
  }
  if (!m_szScoreStuId.IsEmpty()) {
    sql += std::format(" and student_id={} ", m_szScoreStuId.GetBuffer());
  }
  if (!m_szScoreNum.IsEmpty()) {
    sql += std::format(" and score={} ", m_szScoreNum.GetBuffer());
  }
  Package packet(C2S_SELECT, m_qudp.m_sockChannel.m_nConvId,
    m_qudp.m_sockChannel.m_nNextSendSeq++, (char*)sql.c_str(), sql.size() + 1);
  ClearListCtrl();
  m_qudp.SendPackage(packet);

}



/*
学生表的增删改查
*/
void CStudentInfoClientDlg::OnBnClickedStuInsert()
{
  UpdateData(true);
  if (!m_szStuId.IsEmpty() && !m_szStuName.IsEmpty() && !m_szStuClassId.IsEmpty()) {
    std::string sql = std::format("insert into student (student_id, student_name, class_id) \
        values ({}, '{}', {});", m_szStuId.GetBuffer(), 
      m_szStuName.GetBuffer(), m_szStuClassId.GetBuffer());

    Package packet(C2S_INSERT, m_qudp.m_sockChannel.m_nConvId,
      m_qudp.m_sockChannel.m_nNextSendSeq++, (char*)sql.c_str(), sql.size() + 1);
    m_qudp.SendPackage(packet);
  }
  else {
    AfxMessageBox("字段不能为空");
  }
}


void CStudentInfoClientDlg::OnBnClickedStuDelete()
{
  UpdateData(true);
  if (!m_szStuId.IsEmpty()) {
    std::string sql = std::format("delete from student where student_id={} ;",
      m_szStuId.GetBuffer());
    Package packet(C2S_DELETE, m_qudp.m_sockChannel.m_nConvId,
      m_qudp.m_sockChannel.m_nNextSendSeq++, (char*)sql.c_str(), sql.size() + 1);
    m_qudp.SendPackage(packet);
  }
  else {
    AfxMessageBox("字段不能为空");
  }
}


void CStudentInfoClientDlg::OnBnClickedStuUpdate()
{
  UpdateData(true);
  std::string sql = "update student set";
  if (!m_szStuClassId.IsEmpty()) {
    sql += std::format("  class_id={}, ", m_szStuClassId.GetBuffer());
  }
  if (!m_szStuId.IsEmpty()) {
    sql += std::format("  student_id={}, ", m_szStuId.GetBuffer());
  }
  if (!m_szStuName.IsEmpty()) {
    sql += std::format("  student_name='{}', ", m_szStuName.GetBuffer());
  }
  if (m_szStuClassId.IsEmpty() && m_szStuId.IsEmpty() && m_szStuName.IsEmpty()) {
    AfxMessageBox("字段不能全为空");
    return;
  }
  int index = sql.rfind(',');
  sql.replace(index, index + 1, " ");
  sql += std::format(" where id={}", atoi(m_listCtrlData.GetItemText(m_nSelectItem, 0)));
  Package packet(C2S_UPDATE, m_qudp.m_sockChannel.m_nConvId,
    m_qudp.m_sockChannel.m_nNextSendSeq++, (char*)sql.c_str(), sql.size() + 1);
  m_qudp.SendPackage(packet);

#if 0
  if (!m_szStuClassId.IsEmpty() && !m_szStuId.IsEmpty()) {
    std::string sql = std::format("update student set class_id={}, \
      student_id={}, student_name='{}' where id={};",
      m_szStuClassId.GetBuffer(), m_szStuId.GetBuffer(), m_szStuName.GetBuffer(),
      atoi(m_listCtrlData.GetItemText(m_nSelectItem, 0)));
    Package packet(C2S_UPDATE, m_qudp.m_sockChannel.m_nConvId,
      m_qudp.m_sockChannel.m_nNextSendSeq++, (char*)sql.c_str(), sql.size() + 1);
    m_qudp.SendPackage(packet);
    //m_nStudentId = -1;
  }
  else {
    AfxMessageBox("字段不能为空");
  }
#endif // 0

}


void CStudentInfoClientDlg::OnBnClickedStuSelect()
{
  UpdateData(true);
  std::string sql = "select * from student where 1=1 ";
  if (!m_szStuClassId.IsEmpty()) {
    sql += std::format(" and class_id={} ", m_szStuClassId.GetBuffer());
  }
  if (!m_szStuId.IsEmpty()) {
    sql += std::format(" and student_id={} ", m_szStuId.GetBuffer());
  }
  if (!m_szStuName.IsEmpty()) {
    sql += std::format(" and student_name='{}' ", m_szStuName.GetBuffer());
  }

  Package packet(C2S_SELECT, m_qudp.m_sockChannel.m_nConvId,
    m_qudp.m_sockChannel.m_nNextSendSeq++, (char*)sql.c_str(), sql.size() + 1);
  ClearListCtrl();
  m_qudp.SendPackage(packet);
}




//清空列表控件
void CStudentInfoClientDlg::ClearListCtrl()
{

  //清空列表数据
  m_listCtrlData.DeleteAllItems();
  int nColumnCount = m_listCtrlData.GetHeaderCtrl()->GetItemCount();
  for (int i = 0; i < nColumnCount; i++) {
    m_listCtrlData.DeleteColumn(0);
  }
}


void CStudentInfoClientDlg::OnBnClickedQuery()
{
  UpdateData(true);
  string sql;
  string table;
  string where;
  sql += "select ";
  if (m_checkClassId) {
    sql += " c.class_id, ";
    if(!m_szClassId.IsEmpty()) where += std::format(" c.class_id = {} and ", m_szClassId.GetBuffer());
  }
  if (m_checkClassName) {
    sql += " c.class_name, ";
    if (!m_szClassName.IsEmpty()) where += std::format(" c.class_name = {} and ", m_szClassName.GetBuffer());
  }
  if (m_checkClassId || m_checkClassName) {
    table += " class as c, ";
    
  }


  if (m_checkCourseId) {
    sql += " co.course_id, ";
    if (!m_szCourseId.IsEmpty()) where += std::format(" co.course_id = {} and ", m_szCourseId.GetBuffer());
  }
  if (m_checkCourseName) {
    sql += " co.course_name, ";
    if (!m_szCourseName.IsEmpty()) where += std::format(" co.course_name = {} and ", m_szCourseName.GetBuffer());
  }
  if (m_checkCourseId || m_checkCourseName) {
    table += " course as co, ";
  }

  if (m_checkStuId) {
    sql += " stu.student_id, ";
    if (!m_szStuId.IsEmpty()) where += std::format(" stu.student_id = {} and ", m_szStuId.GetBuffer());
  }
  if (m_checkStuName) {
    sql += " stu.student_name, ";
    if (!m_szStuName.IsEmpty()) where += std::format(" stu.student_name = '{}' and ", m_szStuName.GetBuffer());
  }
  if (m_checkStuId || m_checkStuName) {
    table += " student as stu, ";
  }

  if (m_checkCourseNum) {
    sql += " sc.score, ";
    if (!m_szScoreNum.IsEmpty()) where += std::format(" sc.score = {} and ", m_szScoreNum.GetBuffer());
  }

  if (m_checkCourseNum) {
    table += " score as sc, ";
  }
  int index = sql.rfind(',');
  sql.replace(index, index + 1, " ");
  

  sql += " from ";

  index = table.rfind(",");
  if (index > 0) {
    table.replace(index, index + 1, " ");
    sql += table;
  }


  index = where.rfind("and");
  if (index > 0) {
    where.replace(index, index + 3, " ");
    sql += " where ";
    sql += where;
  }



  Package packet(C2S_SELECT, m_qudp.m_sockChannel.m_nConvId,
    m_qudp.m_sockChannel.m_nNextSendSeq++, (char*)sql.c_str(), sql.size() + 1);
  ClearListCtrl();
  m_qudp.SendPackage(packet);
}


void CStudentInfoClientDlg::OnDelete()
{
  // TODO: 在此添加命令处理程序代码
}


void CStudentInfoClientDlg::OnRclickListData(NMHDR* pNMHDR, LRESULT* pResult)
{
  LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
  CMenu menu;
  menu.LoadMenu(MN_DELETE);
  CPoint point;
  ::GetCursorPos(&point);
  ScreenToClient(&point);
  ClientToScreen(&point);
  //menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN, point.x, point.y, this);
  *pResult = 0;
}
