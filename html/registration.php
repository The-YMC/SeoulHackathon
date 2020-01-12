<?php

    include('dbcon.php');
    include('check.php');
   
    if (is_login()){

        if ($_SESSION['user_id'] == 'admin' && $_SESSION['is_admin']==1)
            header("Location: admin.php");
        else
            header("Location: carnumber.php");
    }


function validatePassword($password){
	//Begin basic testing
	if(strlen($password) < 8 || empty($password)) {
		return 0;//Returns 0 if: password is too short (<8 characters) OR doesn't exist.
	}
	if((strlen($password) > 48)) {
		return 0;//Returns 0 if: password is too long (>48 characters)
	}
	//End basic length tests
	
	//Begin more advanced testing
	
	if(preg_match('/[A-Z]/',$password) == (0 || false)){
		return 1;//Returns 1 if: password does NOT contain upper case letters
	}
	if(!preg_match('/[\d]/',$password) != (0 || false)){
		return 2;//Returns 2 if: password does NOT contain digits
	}
	if(preg_match('/[\W]/',$password) == (0 || false)){
		return 3;//Returns 3 if: password does NOT contain any special characters
	}
	return true;
}

	
        if( ($_SERVER['REQUEST_METHOD'] == 'POST') && isset($_POST['submit']))
	{
 
        foreach ($_POST as $key => $val)
        {
            if(preg_match('#^__autocomplete_fix_#', $key) === 1){
                $n = substr($key, 19);
                if(isset($_POST[$n])) {
                    $_POST[$val] = $_POST[$n];
            }
        }
        } 

		$username=$_POST['newusername'];
		$password=$_POST['newpassword'];
		$confirmpassword=$_POST['newconfirmpassword'];
		$phonenumber=$_POST['newphonenumber'];

             //   if (!validatePassword($password)){
	//		$errMSG = "잘못된 패스워드";
          //      }

                if ($_POST['newpassword'] != $_POST['newconfirmpassword']) {
                        $errMSG = "패스워드가 일치하지 않습니다.";
                }

		if(empty($username)){
			$errMSG = "아이디를 입력하세요.";
		}
		else if(empty($password)){
			$errMSG = "패스워드를 입력하세요.";
		}
		else if(empty($phonenumber)){
			$errMSG = "전화번호를 입력하세요.";
		} 

                try { 
                    $stmt = $con->prepare('select * from users where username=:username');
                    $stmt->bindParam(':username', $username);
                    $stmt->execute();

               } catch(PDOException $e) {
                    die("Database error: " . $e->getMessage()); 
               }

               $row = $stmt->fetch();
               if ($row){
                    $errMSG = "이미 존재하는 아이디입니다.";
               }
			   
			   try { 
                    $stmt = $con->prepare('select * from users where phonenumber=:phonenumber');
                    $stmt->bindParam(':phonenumber', $phonenumber);
                    $stmt->execute();

               } catch(PDOException $e) {
                    die("Database error: " . $e->getMessage()); 
               }

               $row = $stmt->fetch();
               if ($row){
                    $errMSG = "이미 존재하는 전화번호입니다.";
               }



		if(!isset($errMSG))
		{
                   try{
			$stmt = $con->prepare('INSERT INTO users(username, password, phonenumber, salt) VALUES(:username, :password, :phonenumber, :salt)');
			$stmt->bindParam(':username',$username);
                        $salt = bin2hex(openssl_random_pseudo_bytes(32));
                        $encrypted_password = base64_encode(encrypt($password, $salt));
                        $stmt->bindParam(':password', $encrypted_password);
			$stmt->bindParam(':phonenumber',$phonenumber);		
			$stmt->bindParam(':salt',$salt);

			if($stmt->execute())
			{
				$successMSG = "새로운 사용자를 추가했습니다.";
			}
			else
			{
				$errMSG = "사용자 추가 에러";
			}
                     } catch(PDOException $e) {
                        die("Database error: " . $e->getMessage()); 
                     }



		}


	}
	
include('head.php');
?>


<div class="container">
	<div>
	<h1 class="h2" align="center">&nbsp; 새로운 사용자 추가</h1><hr>
    </div>
	<?php
	if(isset($errMSG)){
			?>
            <div class="alert alert-danger">
            <span class="glyphicon glyphicon-info-sign"></span> <strong><?php echo $errMSG; ?></strong>
            </div>
            <?php
	}
	else if(isset($successMSG)){
		?>
        <div class="alert alert-success">
              <strong><span class="glyphicon glyphicon-info-sign"></span> <?php echo $successMSG; header('refresh:0');?></strong>
        </div>
        <?php	
			sleep(1);
			header("Location:index.php");
	}
	?>   

<form id="form" method="post" enctype="multipart/form-data" class="form-horizontal" style="margin: 0 300px 0 300px;border: solid 1px;border-radius:4px">
	<table class="table table-responsive">
    <tr>
        <? $r1 = rmd5(rand().mocrotime(TRUE)); ?>
    	<td><label class="control-label">아이디</label></td>
        <td><input class="form-control" type="text" name="<? echo $r1; ?>" placeholder="아이디를 입력하세요." autocomplete="off" readonly 
    onfocus="this.removeAttribute('readonly');" />
            <input type="hidden" name="__autocomplete_fix_<? echo $r1; ?>" value="newusername" /> 

        </td>
    </tr>
    <tr>
        <? $r2 = rmd5(rand().mocrotime(TRUE)); ?>
    	<td><label class="control-label">패스워드</label></td>
        <td>
            <input class="form-control" type="password" name="<? echo $r2; ?>"  placeholder="패스워드를 입력하세요" autocomplete="off" readonly 
                   onfocus="this.removeAttribute('readonly');" />
            <input type="hidden" name="__autocomplete_fix_<? echo $r2; ?>" value="newpassword" /> 
        </td>
    </tr>
    <tr>
        <? $r3 = rmd5(rand().mocrotime(TRUE)); ?>
    	<td><label class="control-label">패스워드 확인</label></td>
        <td>
            <input class="form-control" type="password" name="<? echo $r3; ?>"  placeholder="패스워드를 다시 한번 입력하세요" autocomplete="off" readonly 
                   onfocus="this.removeAttribute('readonly');" />
            <input type="hidden" name="__autocomplete_fix_<? echo $r3; ?>" value="newconfirmpassword" /> 
        </td>
    </tr>

    <tr>
        <? $r4 = rmd5(rand().mocrotime(TRUE)); ?>
    	<td><label class="control-label">전화번호</label></td>
        <td><input class="form-control" type="text" name="<? echo $r4; ?>" placeholder="전화번호을 입력하세요" autocomplete="off" readonly 
    onfocus="this.removeAttribute('readonly');" />
            <input type="hidden" name="__autocomplete_fix_<? echo $r4; ?>" value="newphonenumber" /> 
        </td>
    </tr>
    <tr>
        <td colspan="2" align="center">
        <button type="submit" name="submit"  class="btn btn-primary"><span class="glyphicon glyphicon-floppy-save"></span>&nbsp; 저장</button>
        </td>
    </tr>
    </table>
</form>
</div>
</body>
</html>
